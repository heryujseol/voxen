#pragma once

#include <vector>

#include "Structure.h"

namespace MeshGenerator {

	static void CreateCrossInstanceMesh(
		std::vector<InstanceVertex>& vertices, std::vector<uint32_t>& indices)
	{
		std::vector<Vector3> positions;
		std::vector<Vector3> normals;
		std::vector<Vector2> texcoords;

		// ¹æÇâ '/'
		positions.push_back(Vector3(-0.5f, 0.5f, -0.5f));
		positions.push_back(Vector3(0.5f, 0.5f, 0.5f));
		positions.push_back(Vector3(0.5f, -0.5f, 0.5f));
		positions.push_back(Vector3(-0.5f, -0.5f, -0.5f));
		Vector3 normal = Vector3(0.5f, 0.0f, -0.5f);
		normal.Normalize();
		normals.push_back(normal);
		normals.push_back(normal);
		normals.push_back(normal);
		normals.push_back(normal);
		texcoords.push_back(Vector2(0.0f, 0.0f));
		texcoords.push_back(Vector2(1.0f, 0.0f));
		texcoords.push_back(Vector2(1.0f, 1.0f));
		texcoords.push_back(Vector2(0.0f, 1.0f));

		// ¹æÇâ '\'
		positions.push_back(Vector3(-0.5f, 0.5f, 0.5f));
		positions.push_back(Vector3(0.5f, 0.5f, -0.5f));
		positions.push_back(Vector3(0.5f, -0.5f, -0.5f));
		positions.push_back(Vector3(-0.5f, -0.5f, 0.5f));
		normal = Vector3(0.5f, 0.0f, 0.5f);
		normal.Normalize();
		normals.push_back(normal);
		normals.push_back(normal);
		normals.push_back(normal);
		normals.push_back(normal);
		texcoords.push_back(Vector2(0.0f, 0.0f));
		texcoords.push_back(Vector2(1.0f, 0.0f));
		texcoords.push_back(Vector2(1.0f, 1.0f));
		texcoords.push_back(Vector2(0.0f, 1.0f));

		for (int i = 0; i < 8; i++) {
			InstanceVertex v;
			v.position = positions[i];
			v.normal = normals[i];
			v.texcoord = texcoords[i];
			vertices.push_back(v);
		}

		for (int i = 0; i < 8; i += 4) {
			indices.push_back(i);
			indices.push_back(i + 1);
			indices.push_back(i + 2);

			indices.push_back(i);
			indices.push_back(i + 2);
			indices.push_back(i + 3);
		}
	}

	static void CreateFenceInstanceMesh(
		std::vector<InstanceVertex>& vertices, std::vector<uint32_t>& indices)
	{
		std::vector<Vector3> positions;
		std::vector<Vector3> normals;
		std::vector<Vector2> texcoords;

		// À§ '¤Ñ'
		positions.push_back(Vector3(0.5f, 0.5f, 0.25f));
		positions.push_back(Vector3(-0.5f, 0.5f, 0.25f));
		positions.push_back(Vector3(-0.5f, -0.5f, 0.25f));
		positions.push_back(Vector3(0.5f, -0.5f, 0.25f));
		normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
		normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
		normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
		normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
		texcoords.push_back(Vector2(0.0f, 0.0f));
		texcoords.push_back(Vector2(1.0f, 0.0f));
		texcoords.push_back(Vector2(1.0f, 1.0f));
		texcoords.push_back(Vector2(0.0f, 1.0f));

		// ¾Æ·¡ '¤Ñ'
		positions.push_back(Vector3(-0.5f, 0.5f, -0.25f));
		positions.push_back(Vector3(0.5f, 0.5f, -0.25f));
		positions.push_back(Vector3(0.5f, -0.5f, -0.25f));
		positions.push_back(Vector3(-0.5f, -0.5f, -0.25f));
		normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
		normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
		normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
		normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
		texcoords.push_back(Vector2(0.0f, 0.0f));
		texcoords.push_back(Vector2(1.0f, 0.0f));
		texcoords.push_back(Vector2(1.0f, 1.0f));
		texcoords.push_back(Vector2(0.0f, 1.0f));

		// ¿ÞÂÊ '|'
		positions.push_back(Vector3(-0.25f, 0.5f, 0.5f));
		positions.push_back(Vector3(-0.25f, 0.5f, -0.5f));
		positions.push_back(Vector3(-0.25f, -0.5f, -0.5f));
		positions.push_back(Vector3(-0.25f, -0.5f, 0.5f));
		normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
		normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
		normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
		normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
		texcoords.push_back(Vector2(0.0f, 0.0f));
		texcoords.push_back(Vector2(1.0f, 0.0f));
		texcoords.push_back(Vector2(1.0f, 1.0f));
		texcoords.push_back(Vector2(0.0f, 1.0f));
		
		// ¿À¸¥ÂÊ '|'
		positions.push_back(Vector3(0.25f, 0.5f, -0.5f));
		positions.push_back(Vector3(0.25f, 0.5f, 0.5f));
		positions.push_back(Vector3(0.25f, -0.5f, 0.5f));
		positions.push_back(Vector3(0.25f, -0.5f, -0.5f));
		normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
		normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
		normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
		normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
		texcoords.push_back(Vector2(0.0f, 0.0f));
		texcoords.push_back(Vector2(1.0f, 0.0f));
		texcoords.push_back(Vector2(1.0f, 1.0f));
		texcoords.push_back(Vector2(0.0f, 1.0f));

		for (int i = 0; i < 16; i++) {
			InstanceVertex v;
			v.position = positions[i];
			v.normal = normals[i];
			v.texcoord = texcoords[i];
			vertices.push_back(v);
		}

		for (int i = 0; i < 16; i += 4) {
			indices.push_back(i);
			indices.push_back(i + 1);
			indices.push_back(i + 2);

			indices.push_back(i);
			indices.push_back(i + 2);
			indices.push_back(i + 3);
		}
	}

	static void CreateSquareInstanceMesh(
		std::vector<InstanceVertex>& vertices, std::vector<uint32_t>& indices)
	{
		std::vector<Vector3> positions;
		std::vector<Vector3> normals;
		std::vector<Vector2> texcoords;

		// Z+ ¹æÇâ
		positions.push_back(Vector3(-0.5f, 0.5f, 0.5f));
		positions.push_back(Vector3(0.5f, 0.5f, 0.5f));
		positions.push_back(Vector3(0.5f, -0.5f, 0.5f));
		positions.push_back(Vector3(-0.5f, -0.5f, 0.5f));
		normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
		normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
		normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
		normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
		texcoords.push_back(Vector2(0.0f, 0.0f));
		texcoords.push_back(Vector2(1.0f, 0.0f));
		texcoords.push_back(Vector2(1.0f, 1.0f));
		texcoords.push_back(Vector2(0.0f, 1.0f));

		for (int i = 0; i < 4; i++) {
			InstanceVertex v;
			v.position = positions[i];
			v.normal = normals[i];
			v.texcoord = texcoords[i];
			vertices.push_back(v);
		}

		indices.push_back(0);
		indices.push_back(1);
		indices.push_back(2);
		indices.push_back(0);
		indices.push_back(2);
		indices.push_back(3);
	}

	static void SetSquareIndices(std::vector<uint32_t>& indices, uint32_t offset)
	{
		indices.push_back(offset);
		indices.push_back(offset + 1);
		indices.push_back(offset + 2);

		indices.push_back(offset);
		indices.push_back(offset + 2);
		indices.push_back(offset + 3);
	}

	static VoxelVertex SetVoxelVertex(int x, int y, int z, int face, int type)
	{
		//|x:6||y:6||z:6||face:3||type:8|
		return ((uint32_t)x << 23) | ((uint32_t)y << 17) | ((uint32_t)z << 11) |
			   ((uint32_t)face << 8) | (uint32_t)(type);
	}

	static void CreateQuadMesh(std::vector<VoxelVertex>& vertices, std::vector<uint32_t>& indices,
		int x, int y, int z, int merged, int length, int face, int type)
	{
		uint32_t originVertexSize = (uint32_t)vertices.size();

		// order by vertexID for texcoord
		if (face == 0) {															   // left
			vertices.push_back(SetVoxelVertex(x, y + length, z + merged, face, type)); // 0, 0
			vertices.push_back(SetVoxelVertex(x, y + length, z, face, type));		   // 1, 0
			vertices.push_back(SetVoxelVertex(x, y, z, face, type));				   // 1, 1
			vertices.push_back(SetVoxelVertex(x, y, z + merged, face, type));		   // 0, 1
		}
		else if (face == 1) { // right
			vertices.push_back(SetVoxelVertex(x, y + length, z, face, type));
			vertices.push_back(SetVoxelVertex(x, y + length, z + merged, face, type));
			vertices.push_back(SetVoxelVertex(x, y, z + merged, face, type));
			vertices.push_back(SetVoxelVertex(x, y, z, face, type));
		}
		else if (face == 2) { // bottom
			vertices.push_back(SetVoxelVertex(x, y, z, face, type));
			vertices.push_back(SetVoxelVertex(x + merged, y, z, face, type));
			vertices.push_back(SetVoxelVertex(x + merged, y, z + length, face, type));
			vertices.push_back(SetVoxelVertex(x, y, z + length, face, type));
		}
		else if (face == 3) { // top
			vertices.push_back(SetVoxelVertex(x, y, z + length, face, type));
			vertices.push_back(SetVoxelVertex(x + merged, y, z + length, face, type));
			vertices.push_back(SetVoxelVertex(x + merged, y, z, face, type));
			vertices.push_back(SetVoxelVertex(x, y, z, face, type));
		}
		else if (face == 4) { // front
			vertices.push_back(SetVoxelVertex(x, y + length, z, face, type));
			vertices.push_back(SetVoxelVertex(x + merged, y + length, z, face, type));
			vertices.push_back(SetVoxelVertex(x + merged, y, z, face, type));
			vertices.push_back(SetVoxelVertex(x, y, z, face, type));
		}
		else if (face == 5) { // back
			vertices.push_back(SetVoxelVertex(x + merged, y + length, z, face, type));
			vertices.push_back(SetVoxelVertex(x, y + length, z, face, type));
			vertices.push_back(SetVoxelVertex(x, y, z, face, type));
			vertices.push_back(SetVoxelVertex(x + merged, y, z, face, type));
		}

		SetSquareIndices(indices, originVertexSize);
	}

	static void CreateSkyboxMesh(
		std::vector<SkyboxVertex>& vertices, std::vector<uint32_t>& indices, float scale = 1.0f)
	{
		SkyboxVertex v;

		// ¿ÞÂÊ
		v.position = Vector3(-1.0f, 1.0f, 1.0f) * scale;
		vertices.push_back(v);
		v.position = Vector3(-1.0f, 1.0f, -1.0f) * scale;
		vertices.push_back(v);
		v.position = Vector3(-1.0f, -1.0f, -1.0f) * scale;
		vertices.push_back(v);
		v.position = Vector3(-1.0f, -1.0f, 1.0f) * scale;
		vertices.push_back(v);

		// ¿À¸¥ÂÊ
		v.position = Vector3(1.0f, 1.0f, -1.0f) * scale;
		vertices.push_back(v);
		v.position = Vector3(1.0f, 1.0f, 1.0f) * scale;
		vertices.push_back(v);
		v.position = Vector3(1.0f, -1.0f, 1.0f) * scale;
		vertices.push_back(v);
		v.position = Vector3(1.0f, -1.0f, -1.0f) * scale;
		vertices.push_back(v);

		// ¾Æ·§¸é
		v.position = Vector3(-1.0f, -1.0f, -1.0f) * scale;
		vertices.push_back(v);
		v.position = Vector3(1.0f, -1.0f, -1.0f) * scale;
		vertices.push_back(v);
		v.position = Vector3(1.0f, -1.0f, 1.0f) * scale;
		vertices.push_back(v);
		v.position = Vector3(-1.0f, -1.0f, 1.0f) * scale;
		vertices.push_back(v);

		// À­¸é
		v.position = Vector3(-1.0f, 1.0f, 1.0f) * scale;
		vertices.push_back(v);
		v.position = Vector3(1.0f, 1.0f, 1.0f) * scale;
		vertices.push_back(v);
		v.position = Vector3(1.0f, 1.0f, -1.0f) * scale;
		vertices.push_back(v);
		v.position = Vector3(-1.0f, 1.0f, -1.0f) * scale;
		vertices.push_back(v);

		// ¾Õ¸é
		v.position = Vector3(-1.0f, 1.0f, -1.0f) * scale;
		vertices.push_back(v);
		v.position = Vector3(1.0f, 1.0f, -1.0f) * scale;
		vertices.push_back(v);
		v.position = Vector3(1.0f, -1.0f, -1.0f) * scale;
		vertices.push_back(v);
		v.position = Vector3(-1.0f, -1.0f, -1.0f) * scale;
		vertices.push_back(v);

		// µÞ¸é
		v.position = Vector3(1.0f, 1.0f, 1.0f) * scale;
		vertices.push_back(v);
		v.position = Vector3(-1.0f, 1.0f, 1.0f) * scale;
		vertices.push_back(v);
		v.position = Vector3(-1.0f, -1.0f, 1.0f) * scale;
		vertices.push_back(v);
		v.position = Vector3(1.0f, -1.0f, 1.0f) * scale;
		vertices.push_back(v);

		for (uint32_t i = 0; i < 24; i += 4) {
			SetSquareIndices(indices, i);
		}
	}

	static void CreateCloudMesh(std::vector<CloudVertex>& vertices, std::vector<uint32_t>& indices,
		int x, int y, int z, bool x_n, bool x_p, bool y_n, bool y_p, bool z_n, bool z_p)
	{
		uint32_t originVertexSize = (uint32_t)vertices.size();
		uint32_t faceCount = 0;
		CloudVertex vertex;

		// ¿ÞÂÊ
		if (x_n) {
			vertex.face = 0;
			vertex.position = Vector3(x + 0.0f, y + 1.0f, z + 1.0f);
			vertices.push_back(vertex);
			vertex.position = Vector3(x + 0.0f, y + 1.0f, z + 0.0f);
			vertices.push_back(vertex);
			vertex.position = Vector3(x + 0.0f, y + 0.0f, z + 0.0f);
			vertices.push_back(vertex);
			vertex.position = Vector3(x + 0.0f, y + 0.0f, z + 1.0f);
			vertices.push_back(vertex);
			faceCount++;
		}

		// ¿À¸¥ÂÊ
		if (x_p) {
			vertex.face = 1;
			vertex.position = Vector3(x + 1.0f, y + 1.0f, z + 0.0f);
			vertices.push_back(vertex);
			vertex.position = Vector3(x + 1.0f, y + 1.0f, z + 1.0f);
			vertices.push_back(vertex);
			vertex.position = Vector3(x + 1.0f, y + 0.0f, z + 1.0f);
			vertices.push_back(vertex);
			vertex.position = Vector3(x + 1.0f, y + 0.0f, z + 0.0f);
			vertices.push_back(vertex);
			faceCount++;
		}

		// ¾Æ·§¸é
		if (y_n) {
			vertex.face = 2;
			vertex.position = Vector3(x + 0.0f, y + 0.0f, z + 0.0f);
			vertices.push_back(vertex);
			vertex.position = Vector3(x + 1.0f, y + 0.0f, z + 0.0f);
			vertices.push_back(vertex);
			vertex.position = Vector3(x + 1.0f, y + 0.0f, z + 1.0f);
			vertices.push_back(vertex);
			vertex.position = Vector3(x + 0.0f, y + 0.0f, z + 1.0f);
			vertices.push_back(vertex);
			faceCount++;
		}


		// À­¸é
		if (y_p) {
			vertex.face = 3;
			vertex.position = Vector3(x + 0.0f, y + 1.0f, z + 1.0f);
			vertices.push_back(vertex);
			vertex.position = Vector3(x + 1.0f, y + 1.0f, z + 1.0f);
			vertices.push_back(vertex);
			vertex.position = Vector3(x + 1.0f, y + 1.0f, z + 0.0f);
			vertices.push_back(vertex);
			vertex.position = Vector3(x + 0.0f, y + 1.0f, z + 0.0f);
			vertices.push_back(vertex);
			faceCount++;
		}


		// ¾Õ¸é
		if (z_n) {
			vertex.face = 4;
			vertex.position = Vector3(x + 0.0f, y + 1.0f, z + 0.0f);
			vertices.push_back(vertex);
			vertex.position = Vector3(x + 1.0f, y + 1.0f, z + 0.0f);
			vertices.push_back(vertex);
			vertex.position = Vector3(x + 1.0f, y + 0.0f, z + 0.0f);
			vertices.push_back(vertex);
			vertex.position = Vector3(x + 0.0f, y + 0.0f, z + 0.0f);
			vertices.push_back(vertex);
			faceCount++;
		}

		// µÞ¸é
		if (z_p) {
			vertex.face = 5;
			vertex.position = Vector3(x + 1.0f, y + 1.0f, z + 1.0f);
			vertices.push_back(vertex);
			vertex.position = Vector3(x + 0.0f, y + 1.0f, z + 1.0f);
			vertices.push_back(vertex);
			vertex.position = Vector3(x + 0.0f, y + 0.0f, z + 1.0f);
			vertices.push_back(vertex);
			vertex.position = Vector3(x + 1.0f, y + 0.0f, z + 1.0f);
			vertices.push_back(vertex);
			faceCount++;
		}

		for (uint32_t i = 0; i < faceCount; ++i) {
			SetSquareIndices(indices, originVertexSize + i * 4);
		}
	}

	static void CreateSampleSquareMesh(
		std::vector<SamplingVertex>& vertices, std::vector<uint32_t>& indices)
	{
		SamplingVertex vertex;

		vertex.position = Vector3(-1.0f, 1.0f, 0.0f);
		vertex.texcoord = Vector2(0.0f, 0.0f);
		vertices.push_back(vertex);

		vertex.position = Vector3(1.0f, 1.0f, 0.0f);
		vertex.texcoord = Vector2(1.0f, 0.0f);
		vertices.push_back(vertex);

		vertex.position = Vector3(1.0f, -1.0f, 0.0f);
		vertex.texcoord = Vector2(1.0f, 1.0f);
		vertices.push_back(vertex);

		vertex.position = Vector3(-1.0f, -1.0f, 0.0f);
		vertex.texcoord = Vector2(0.0f, 1.0f);
		vertices.push_back(vertex);

		SetSquareIndices(indices, 0);
	}
}