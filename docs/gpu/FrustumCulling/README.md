# Frustum Culling

<img width="1914" height="1075" alt="Image" src="https://github.com/user-attachments/assets/e21ac2ee-6d11-42c2-844b-df6d729c19dc" />

<img width="955" height="538" alt="Image" src="https://github.com/user-attachments/assets/2f6eb2be-9c92-4e7d-a966-d44e35db4f52" />

## 1. 개요

Frustum Culling은 카메라의 시야 절두체(frustum) 밖에 있는 청크를 렌더링 대상에서 제외하는 기법이다.
화면에 보이지 않는 청크의 드로우 콜을 제거하여 GPU 부하를 줄이는 핵심 최적화다.
매 프레임 로드된 모든 청크를 대상으로 3종류의 프러스텀 검사를 수행한다.

1. 단순한 카메라에 들어올 청크 렌더 리스트 컬링
2. 쉐도우 맵을 위한 Light 청크 렌더 리스트 컬링
3. 수면 반사를 위한 청크 렌더 리스트 컬링

초기엔 View NDC 박스를 역변환 후 World 기준으로 구성된 Frustum 평면 6개와, 청크 8개의 꼭짓점을 모두 연산하여 결과를 나타냈다.

현재는 수정하여 Gribb-Hartmann Planes 추출하여 AABB로 Frustum Culling을 최적화하였다.

## 2. 도입 동기

복셀 월드에서 렌더 거리 내 로드된 청크 수는 수백 개에 달한다.
이 중 카메라 시야에 실제로 들어오는 청크는 일부에 불과하지만, 컬링 없이 모두 GPU에 보내면 불필요한 버텍스 처리와 래스터라이징이 발생한다.
프러스텀 컬링으로 보이지 않는 청크를 CPU 단계에서 걸러내면, GPU에 전달되는 드로우 콜 수를 대폭 줄일 수 있다.

추가로 섀도우 맵과 수면 반사도 각각 별도의 프러스텀을 가지므로, 이들에 대해서도 독립적인 컬링이 필요하다.

<details>
<summary>Gribb-Hartmann 이전(초기 구현 내용) 내용 보기</summary>

## 3. 초기: 핵심 아이디어

### 3.1 NDC 역변환 기반 프러스텀 구성

초기엔 비효율적이지만 직관적인 역변환 형태의 방식을 채용했다.

디버깅이 쉽고, 기하적으로 이해하기 쉬운 코드를 작성하기 위함이다.

```
NDC 큐브의 8개 꼭짓점 → (View × Projection)⁻¹ 역변환 → 월드 공간 8개 꼭짓점 → 6개 평면 구성
```

NDC 공간의 단위 큐브 꼭짓점을 월드 공간으로 역변환하면 프러스텀의 실제 월드 좌표를 얻고, 이 꼭짓점들로부터 6개 평면을 구성하는 방식이다.

### 3.2 보수적 판정

1. Chunk의 8개 꼭짓점 모두가 어느 한 평면의 바깥에 있어야만 해당 청크를 제외한다.
2. 꼭짓점 하나라도 안쪽에 있으면 보수적으로 통과시킨다.

## 4. 초기: 구현 내용

### 4.1 렌더 리스트 구성 (UpdateRenderChunkList)

매 프레임 `m_chunkMap`의 모든 로드된 청크를 순회하며 3가지 프러스텀 검사를 수행한다.

```
m_chunkMap 순회 (로드 완료 + 비어있지 않은 청크만):
│
├─ [카메라 프러스텀] FrustumCulling(chunkPos, useMirror=false, useShadow=false)
│   → 통과 시 m_renderChunkList에 추가
│
├─ [섀도우 프러스텀] CASCADE_NUM(3)개 캐스케이드 중 하나라도 통과하면
│   FrustumCulling(chunkPos, useShadow=true, index=0~2)
│   → 통과 시 m_renderShadowChunkList에 추가 (break로 중복 방지)
│
└─ [미러 프러스텀] 청크 위치를 미러 평면으로 반사 변환 후
    mirrorChunkPos = Transform(chunkPos, mirrorPlaneMatrix)
    FrustumCulling(mirrorChunkPos, useMirror=true, useShadow=false)
    → 통과 시 m_renderMirrorChunkList에 추가
```

| 렌더 리스트               | 프러스텀 소스                        | 용도                                            |
| ------------------------- | ------------------------------------ | ----------------------------------------------- |
| `m_renderChunkList`       | 카메라 View × Projection             | 메인 렌더링 (Opaque + SemiAlpha + Transparency) |
| `m_renderShadowChunkList` | Light View × Cascade Projection[0~2] | 3개 캐스케이드 섀도우 맵                        |
| `m_renderMirrorChunkList` | 카메라 프러스텀 + 미러 평면 반사     | 수면 반사 렌더링                                |

### 4.2 프러스텀 평면 구성

```cpp
// 1. 역행렬 계산
Matrix invMat;
if (useShadow)
    invMat = (light.GetViewMatrix() * light.GetProjectionMatrixFromCascade(index)).Invert();
else
    invMat = (camera.GetViewMatrix() * camera.GetProjectionMatrix()).Invert();
```

프러스텀 종류에 따라 사용하는 행렬이 다르다.

| 프러스텀            | View 행렬   | Projection 행렬                    |
| ------------------- | ----------- | ---------------------------------- |
| 카메라 / 미러       | Camera View | Camera Perspective Projection      |
| 섀도우 캐스케이드 i | Light View  | Cascade Orthographic Projection[i] |

- cf. 미러는 Camera View를 그대로 사용하고, 물체 `position`이 수면에 대한 반사 행렬에 곱해져 들어온다.

```cpp
// 2. NDC 8개 꼭짓점 → 월드 공간 역변환
worldPos[0] = Transform((-1, +1, 0), invMat)   // Near Top-Left
worldPos[1] = Transform((+1, +1, 0), invMat)   // Near Top-Right
worldPos[2] = Transform((+1, -1, 0), invMat)   // Near Bottom-Right
worldPos[3] = Transform((-1, -1, 0), invMat)   // Near Bottom-Left
worldPos[4] = Transform((-1, +1, 1), invMat)   // Far Top-Left
worldPos[5] = Transform((+1, +1, 1), invMat)   // Far Top-Right
worldPos[6] = Transform((+1, -1, 1), invMat)   // Far Bottom-Right
worldPos[7] = Transform((-1, -1, 1), invMat)   // Far Bottom-Left
```

NDC에서 Z=0은 Near 평면, Z=1은 Far 평면이다 (DirectX 좌표계). 이 8개 점을 역변환하면 월드 공간에서의 프러스텀 꼭짓점이 된다.

```cpp
// 3. 8개 꼭짓점으로 6개 평면 생성
vfPlanes[0] = XMPlaneFromPoints(worldPos[0], worldPos[1], worldPos[2])  // Near
vfPlanes[1] = XMPlaneFromPoints(worldPos[7], worldPos[6], worldPos[5])  // Far
vfPlanes[2] = XMPlaneFromPoints(worldPos[4], worldPos[5], worldPos[1])  // Top
vfPlanes[3] = XMPlaneFromPoints(worldPos[3], worldPos[2], worldPos[6])  // Bottom
vfPlanes[4] = XMPlaneFromPoints(worldPos[4], worldPos[0], worldPos[3])  // Left
vfPlanes[5] = XMPlaneFromPoints(worldPos[1], worldPos[5], worldPos[6])  // Right
```

### 4.3 판정

```cpp
float x = (float)Chunk::CHUNK_SIZE;  // 32
float y = (float)Chunk::CHUNK_SIZE;  // 32
float z = (float)Chunk::CHUNK_SIZE;  // 32
if (useMirror)
    y *= -1;  // 미러 반사 시 Y축 반전
```

청크의 8개의 꼭짓점은 `position`을 원점으로 `(position + 32, position + 32, position + 32)`까지의 축 정렬 박스다.
미러 렌더링 시에는 Y축이 반전되어 `position.y - 32` 방향으로 확장된다.

```cpp
for (int i = 0; i < vfPlanes.size(); ++i) {
    if (PlaneDotCoord(vfPlanes[i], position)                       <= 0) continue; // 한 점이 평면 내부면 다음 평면 판단
    if (PlaneDotCoord(vfPlanes[i], position + (x, 0, 0))           <= 0) continue;
    if (PlaneDotCoord(vfPlanes[i], position + (0, y, 0))           <= 0) continue;
    if (PlaneDotCoord(vfPlanes[i], position + (x, y, 0))           <= 0) continue;
    if (PlaneDotCoord(vfPlanes[i], position + (0, 0, z))           <= 0) continue;
    if (PlaneDotCoord(vfPlanes[i], position + (x, 0, z))           <= 0) continue;
    if (PlaneDotCoord(vfPlanes[i], position + (0, y, z))           <= 0) continue;
    if (PlaneDotCoord(vfPlanes[i], position + (x, y, z))           <= 0) continue;
    return false;  // 8개 꼭짓점 모두 이 평면 바깥 → 프러스텀 밖
}
return true;  // 어느 평면에서도 완전히 밖이 아님 → 프러스텀 안(또는 교차)
```

**판정 로직:**

`XMPlaneDotCoord(plane, point)`는 점이 평면의 안쪽(음수)인지 바깥쪽(양수)인지를 반환한다.

```
PlaneDotCoord 결과:
  ≤ 0  →  점이 프러스텀 안쪽 (또는 평면 위)  →  continue (다음 꼭짓점 검사)
  > 0  →  점이 프러스텀 바깥쪽                →  다음 꼭짓점으로
```

6개 평면 각각에 대해:

1. Chunk의 8개 꼭짓점을 순회
2. **하나라도** 평면 안쪽(≤ 0)이면 → `continue`로 해당 평면 통과 (다음 평면 검사)
3. **8개 모두** 바깥쪽(> 0)이면 → `return false` (프러스텀 밖으로 확정, 컬링)

6개 평면을 모두 통과하면 `return true` (프러스텀 안에 있거나 교차).

</details>

## 3. 핵심 개념

AABB와 Gribb-Hartmann의 결합이 중요하다.

### 3.1 AABB 최적화

Chunk 자체는 축 정렬된 좌표를 가지고 있기 때문에 모든 점에 대한 검사는 비효율적이다.

`n-Vertex / p-Vertex`를 구해서 최적화가 가능하다.

```
n-Vertex: 평면에 대입했을 때(노멀과 내적)의 값이 가장 최소인 위치 - 평면 노멀 방향에 멈
p-Vertex: 평면에 대입했을 때(노멀과 내적)의 값이 가장 최대인 위치 - 평면 노멀 방향에 가까움
```

오개념을 조심해야 하는데, `n-Vertex`는 평면에 가장 가까운 한 점을 구하는게 아니다.

- `n-Vertex`는 단순히 평면에 대입했을 때 가장 작은 값을 고를 뿐이다. (평면 노멀과 위치벡터의 내적의 최소)

`n-Vertex/p-Vertex`를 구하는 방식은 다음과 같다.

```
minPos = (0, 0, 0)
maxPos = minPos + (ChunkSize, ChunkSize, ChunkSize)

// 평면의 노멀벡터
N = (a, b, c)

// n vertex
nVertex.x = (a > 0) ? minPos.x : maxPos.x // 노멀 방향에 반대임
nVertex.y = (b > 0) ? minPos.y : maxPos.y
nVertex.z = (c > 0) ? minPos.z : maxPos.z

// p vertex
pVertex.x = (a > 0) ? maxPos.x : minPos.x // 노멀 방향에 가까움
pVertex.y = (b > 0) ? maxPos.y : minPos.y
pVertex.z = (c > 0) ? maxPos.z : minPos.z
```

### 3.2 Gribb-Hartmann 평면 추출

어느 한 점이 View Frustum에 들어오는지는 다음과 같다.

```
P * [VP Matrix]
P * [col0, col1, col2, col3] ==> [x_c, y_c, z_c, w_c]

NDC-x: -1 <= x_c/w_c <= 1
NDC-y: -1 <= y_c/w_c <= 1
NDC-z:  0 <= z_c/w_c <= 1
```

이 때, NDC도 가지말고 Clip Space에서 연산이 충분히 가능하다.

```
P * [col0, col1, col2, col3] ==> [x_c, y_c, z_c, w_c]

clip-x: -w_c <= x_c <= w_c
clip-y: -w_c <= y_c <= w_c
clip-z:    0 <= z_c <= w_c

leftSide 판정:  x_c + w_c >= 0 (내부)
rightSide 판정: w_c - x_c >= 0 (내부)
...

```

또한 실제로 `x_c` 혹은 `y_c`와 같은 연산을 직접 계산할 필요도 없다. 해당 값은 결국 `P`가 `col-N` 벡터와 곱해진 결과이다.

```
leftSide 판정: x_c + w_c >= 0 (내부)

x_c == P * col0
w_c == P * col3

x_c + w_c == P * (col0 + col3)

leftSide 판정: P * (col0 + col3) >= 0  (내부)
```

leftSide와 마찬가지로 다른 Side에 대해서도 이미 값을 계산해놓는다.
이 때, `P`와 내적되는 오른쪽 column Vector를 평면으로 본다. 이것이 **Gribb-Hartmann Plane**이 된다.

즉, 미리 Matrix를 이용하여 Side에 맞는 **Gribb-Hartmann Plane** 구성한 후 임의의 점 하나를 평면에 대입하여 부등호 연산을 하면 위치가 판단된다.

### 3.3 AABB + Gribb-Hartmann 평면 결합

AABB로 nVertex나 pVertex를 구하고, Gribb-Hartmann 평면에 대입하기만 하면 Frustum Culling을 진행할 수 있게 된다.

이 때, Gribb-Hartmann 평면이 어느 방향을 바라보는지에 따라 n-Vertex 를 사용할지 p-Vertex를 사용할지 잘 구분해여 사용해야 한다.

나의 프로젝트에서는 p-Vertex를 사용하여 Culling을 진행하였다.

## 4. 구현 내용

### 4.1 평면 사전 추출

이전에는 Frustum Culling 호출마다 역행렬을 구해서 곱하는 비효율의 연속이였고, useMirror, useShadow에 따라 구분하여 함수가 좋지 못했다.

그래서 Frustum Culling 호출 전에 camera, mirror, shadow에 따른 Gribb-Hartmann Planes 미리 구성하고 호출하였다.

```
void ChunkManager::UpdateRenderChunkList(Camera& camera, const Light& light)
{
	/*
	* Frustum Culling에 사용할 Gribb-Hartmann 평면 추출
	*/
    // 일반 카메라: Gribb-Hartmann 평면 추출
	Matrix cameraViewProjMatrix = camera.GetViewMatrix() * camera.GetProjectionMatrix();
	std::array<Vector4, 6> cameraGribbHartmannPlanes;
	GetGribbHartmannPlanes(cameraViewProjMatrix, cameraGribbHartmannPlanes);

    // Shadow: Gribb-Hartmann 평면 추출
	std::vector<std::array<Vector4, 6>> cascadeShadowGribbHartmannPlanes(Light::CASCADE_LEVEL);
	for (int i = 0; i < Light::CASCADE_LEVEL; ++i) {
		Matrix cascadeShadowViewProjMatrix =
			light.GetShadowViewMatrix() * light.GetProjectionMatrixFromCascade(i);

		GetGribbHartmannPlanes(
			cascadeShadowViewProjMatrix, cascadeShadowGribbHartmannPlanes[i]);
	}

	for (auto& p : m_chunkMap) {
		Chunk* chunk = p.second;

        // ... 검사

		Vector3 chunkPos = chunk->GetPosition();

		// ... 종류별 Frustum Culling() 호출
        if (Frustum Culling(chunkPos, cameraGribbHartmannPlanes))
            ...
	}
}
```

### 4.2 Gribb-Hartmann 평면 추출

```
void ChunkManager::GetGribbHartmannPlanes(const Matrix& vpm, std::array<Vector4, 6>& outPlanes)
{
	Vector4 colVectors[4];
	for (int col = 0; col < 4; ++col) {
		colVectors[col] = Vector4(vpm.m[0][col], vpm.m[1][col], vpm.m[2][col], vpm.m[3][col]);
	}

	outPlanes[0] = colVectors[0] + colVectors[3]; // left
	outPlanes[1] = colVectors[3] - colVectors[0]; // right
	outPlanes[2] = colVectors[1] + colVectors[3]; // bottom
	outPlanes[3] = colVectors[3] - colVectors[1]; // top
	outPlanes[4] = colVectors[2];				  // near
	outPlanes[5] = colVectors[3] - colVectors[2]; // far
}
```

### 4.3 Frustum Culling 호출

실제 Frustum Culling은 매우 짧아졌다.

평면의 노멀이 Frustum Culling 안쪽을 가리키기 때문에 내적의 결과가 가장 큰 `pVertex`를 이용하여 판단한다.

```
bool ChunkManager::FrustumCulling(
	Vector3 position, const std::array<Vector4, 6>& gribbHartmannPlanes)
{
	Vector3 minPos = position;
	Vector3 maxPos = position + Vector3(Chunk::CHUNK_SIZE);

	for (int i = 0; i < 6; ++i) {
		float a = gribbHartmannPlanes[i].x;
		float b = gribbHartmannPlanes[i].y;
		float c = gribbHartmannPlanes[i].z;
		float d = gribbHartmannPlanes[i].w;

		Vector3 pVertex;
		pVertex.x = (a > 0) ? maxPos.x : minPos.x;
		pVertex.y = (b > 0) ? maxPos.y : minPos.y;
		pVertex.z = (c > 0) ? maxPos.z : minPos.z;

		if (a * pVertex.x + b * pVertex.y + c * pVertex.z + d < 0)
			return false;
	}

	return true;
}
```

## 5. 3가지 프러스텀 비교

### 5.1 Camera

```
카메라 위치에서 원근 투영(Perspective)으로 형성되는 사다리꼴 절두체.
가까운 곳은 좁고 먼 곳은 넓어진다.

      ╱‾‾‾‾‾‾‾‾‾╲
     ╱           ╲
    ╱    카메라    ╲
   ╱    시야 영역    ╲
   ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
 Near              Far
```

입력: `camera.GetViewMatrix() × camera.GetProjectionMatrix()`

### 5.2 Shadow Light

```
각 캐스케이드는 직교 투영(Orthographic)으로 형성되는 직육면체.
빛 방향을 기준으로 공간을 분할한다.

   ┌──────┐  ┌──────────┐  ┌────────────────┐
   │ C0   │  │   C1     │  │      C2        │
   │ 가까움│  │  중간    │  │     먼 곳       │
   └──────┘  └──────────┘  └────────────────┘

for (int i = 0; i < Light::CASCADE_LEVEL; ++i) {
	if (FrustumCulling(chunkPos, cascadeShadowGribbHartmannPlanes[i])) {
		m_renderShadowChunkList.push_back(chunk);
		break;
	}
}
```

입력: `light.GetViewMatrix() × light.GetProjectionMatrixFromCascade(i)` (i = 0, 1, 2)

3개 캐스케이드 중 하나라도 통과하면 리스트에 추가한다. 캐스케이드 간 중복 방지를 위해 첫 번째 통과 시 `break`한다.

### 5.3 Mirror World

카메라는 그대로지만, Chunk Position 기준점이 Plane에 따라 뒤집힌다.
그래서 Chunk Position의 기준점을 내려서 Frustum Culling을 실행시킨다.

```
Vector3 mirrorChunkPos = Vector3::Transform(chunkPos, camera.GetMirrorPlaneMatrix());
mirrorChunkPos.y -= Chunk::CHUNK_SIZE;
if (FrustumCulling(mirrorChunkPos, cameraGribbHartmannPlanes)) {
	m_renderMirrorChunkList.push_back(chunk);
}
```

## 6. 결과

초기의 역변환 형태의 Frustum Culling은 평균 `1.0ms`가 걸릴만큼 느렸지만, Gribb-Hartmann AABB로 인해 평균 `0.2ms`로 속도가 매우 단축되었다.

## 7. 회고

- 잊어버린 평면과 점의 관계를 파악할 수 있는 챕터였다
- 초기엔 역변환 형식의 직관적인 Culling을 진행했지만, Gribb-Hartmann 평면 추출 방식으로 수정하여 속도가 매우 단축되었다.
- Gribb-Hartmann으로 추출된 평면의 노멀 방향이 내가 초기에 구성한 역변환 ViewFrustum 평면의 방향과 달라서 문제가 있었지만 해결했다.
  - left side: `p*(col0 + col3) >= 0`를 만족하는 것은 내부라는 것이고, left 평면이 ViewFrustum 안쪽을 가르킨다고 판단 해야한다.

## 8. 나아가 (적용한 AABB와 Gribb-Hartmann 개념 및 Viewer 구현 내용)

### 평면의 방정식

```cpp
// 평면의 노멀
N = (a, b, c) // 평면의 노멀

// 평면의 한점
Q = (x1, y1, z1)

// 평면의 임의의 점
P = (x, y, z)

// 수직
N * (P - Q) = 0

// 수식
N*P - N*Q = 0
ax + by + cz - (ax1 + by1 + cz1) = 0
ax + by + cz + d = 0 (d = -N*Q)

// 평면 위의 점인가 판단
// 값 대입
 0  : 평면 위의 점
음수: 노멀벡터 반대 방향 (P-Q 벡터가 노멀벡터 반대 방향이라 내적 값이 음수임)
양수: 노멀벡터 방향 (P-Q 벡터가 노멀벡터 방향이라 내적 값이 양수임)
```

---

### AABB

- **Axis-Aligned Bounding Box**

#### AABB에서의 두 점과 n/p-vertex로 최적화

- AABB의 좌표로 최대최소점을 구하고 그것을 평면의 노멀벡터의 음양부호에 따라 n-vertex / p-vertex를 구할 수 있음
  - **n-vertex** : 평면 노멀과 내적의 결과가 최소
  - **p-vertex** : 평면 노멀과 내적의 결과가 최대
- n-vertex, p-vertex 만을 가지고 충분히 프러스텀의 내외부 판단을 할 수 있음

```cpp
minPos = (0, 0, 0)
maxPos = minPos + (ChunkSize, ChunkSize, ChunkSize)

// 평면의 노멀벡터
N = (a, b, c)

// n vertex
nVertex.x = (a > 0) ? minPos.x : maxPos.x
nVertex.y = (b > 0) ? minPos.y : maxPos.y
nVertex.z = (c > 0) ? minPos.z : maxPos.z

// p vertex
pVertex.x = (a > 0) ? maxPos.x : minPos.x
pVertex.y = (b > 0) ? maxPos.y : minPos.y
pVertex.z = (c > 0) ? maxPos.z : minPos.z

// frustum culling
for (int i = 0; i < vfPlanes.size(); ++i)
{
    if (XMVectorGetX(XMPlaneDotCoord(vfPlanes[i], nVertex)) <= 0) continue;
    ...
}
```

---

### Gribb-Hartmann Culling

- Clip Space 부호를 활용한 방식
  - 결국 메쉬의 Position이 NDC 좌표에 들어와야하고, NDC 좌표 이전 ClipSpace에 대한 부호 검사를 진행

```cpp
// NDC
-1 <= x <= 1
-1 <= y <= 1
 0 <= z <= 1

// clip space
-w <= x <= w
-w <= y <= w
 0 <= z <= w

// P * ViewProj
    |                     |
P * | col0 col1 col2 col3 | => [P*col0, P*col1, P*col2, P*col3] => [x_c, y_c, z_c, w_c]
    |                     |

// 부호 판단
-w_c <= x_c <= w_c
-w_c <= y_c <= w_c
  0  <= z_c <= w_c

x_c + w_c >= 0 // left
w_c - x_c >= 0 // right
y_c + w_c >= 0 // bottom
w_c - y_c >= 0 // top
      z_c >= 0 // near
w_c - z_c >= 0 // far

// left 예시
x_c + w_c >= 0
P*col0 + P*col3 >= 0
P*(col0 + col3) >= 0
// col0 + col3 이 left의 평면 계수가 됨
```

- 여기서 중요!
  - P와 내적되는 평면(ex. `col0 + col3`)의 노멀은 프러스텀 안쪽을 가르키는 방향임에 명심해야 함

---

### Gribb-Hartmann Culling + AABB 최적화

```cpp
// VP에서 column 추출 (row-major 기준)
Vector4 col0, col1, col2, col3;  // VP의 각 열

// Gribb-Hartmann으로 6개 평면 계수 구성
Vector4 planes[6] = {
    col0 + col3,   // Left
    col3 - col0,   // Right
    col1 + col3,   // Bottom
    col3 - col1,   // Top
    col2,          // Near
    col3 - col2,   // Far
};

for (int i = 0; i < 6; ++i) {
    float a = planes[i].x;
    float b = planes[i].y;
    float c = planes[i].z;
    float d = planes[i].w;

    // p-vertex 선택
    Vector3 p;
    p.x = (a > 0) ? max.x : min.x;
    p.y = (b > 0) ? max.y : min.y;
    p.z = (c > 0) ? max.z : min.z;

    // XMPlaneDotCoord 없이 직접 대입
    if (a*n.x + b*n.y + c*n.z + d < 0)
        return false;  // 컬링
}
return true;
```

---

### Frustum Viewer

VS에서 Camera.ConstantBuffer에 대한 View를 적절히 바꿔줌

```cpp
// Camera
// Debug Camera for Frustum Culling
m_cullingViewerOffsetPos = Vector3(0.0f, 128.0f, -128.0f);
m_cullingViewerPos = m_eyePos + m_cullingViewerOffsetPos;

Quaternion qPitch = Quaternion(
    Vector3(1.0f, 0.0f, 0.0f) * sinf(XM_PIDIV2 * 0.25f), cosf(XM_PIDIV2 * 0.25f));
m_cullingViewerForward = Vector3::Transform(Vector3(0.0f, 0.0f, 1.0f), Matrix::CreateFromQuaternion(qPitch));
m_cullingViewerUp = Vector3::Transform(Vector3(0.0f, 1.0f, 0.0f), Matrix::CreateFromQuaternion(qPitch));

m_constantData.view = XMMatrixLookToLH(m_cullingViewerPos, m_cullingViewerForward, m_cullingViewerUp);
m_constantData.view = m_constantData.view.Transpose();

if (!DXUtils::CreateConstantBuffer(m_cullingViewerConstantBuffer, m_constantData)) {
    std::cout << "failed create debug camera constant buffer" << std::endl;
    return false;
}

// App::RenderFrustumCullingViewer()
Graphics::context->VSSetConstantBuffers(
		8, 1, m_camera.m_cullingViewerConstantBuffer.GetAddressOf());
```

Chunk Render는 단순히 LOD Basic으로 렌더링 -> SRV 그대로 사용

```cpp
std::vector<ID3D11ShaderResourceView*> ppSRVs;
ppSRVs.push_back(Graphics::blockAtlasMapSRV.Get());
ppSRVs.push_back(Graphics::normalAtlasMapSRV.Get());
ppSRVs.push_back(Graphics::merAtlasMapSRV.Get());
ppSRVs.push_back(Graphics::grassColorMapSRV.Get());
ppSRVs.push_back(Graphics::foliageColorMapSRV.Get());
ppSRVs.push_back(Graphics::climateMapSRV.Get());
Graphics::context->PSSetShaderResources(0, (UINT)ppSRVs.size(), ppSRVs.data());

ChunkManager::GetInstance()->RenderBasicAlbedo();
```

ViewFrustum을 NDC로 메쉬를 만들고 Camera의 Inv(ViewProj)을 VS에서 변환하여 WorldPosition으로 역변환하여 사용
그것을 현재 Viewer 카메라에 맞게 View, Proj 변환

```hlsl
vsOutput main(vsInput input)
{
    vsOutput output;

    float4 frustumNDCPosition = float4(input.position, 1.0);

    float4 frustumViewPosition = mul(frustumNDCPosition, invProj);
    frustumViewPosition.xyz /= frustumViewPosition.w;

    float4 frustumWorldPosition = mul(frustumViewPosition, invView);

    output.posProj = mul(frustumWorldPosition, view);
    output.posProj = mul(output.posProj, proj);

    output.color = float3(1.0, 0.0, 0.0);

    return output;
}
```
