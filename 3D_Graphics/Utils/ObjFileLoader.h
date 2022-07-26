#pragma once

#include <string>
#include <fstream>
#include <ErrorHandle/StandardException.h>
#include <imgui/imgui.h>

typedef struct
{
	int vertexCount;
	int textureCount;
	int normalCount;
	int faceCount;
	bool isTriangulated;
} FileInfo;

typedef struct
{
	float x, y, z;
} Float3Type;

typedef struct
{
	int vIndex1, vIndex2, vIndex3, vIndex4;
	int tIndex1, tIndex2, tIndex3, tIndex4;
	int nIndex1, nIndex2, nIndex3, nIndex4;
} FaceType;

FileInfo GetModelInfo(std::string filePath)
{
	std::ifstream file;
	std::string input;
	FileInfo info = { 0,0,0,0,false };
	bool checked = false;

	// 파일 열기
	file.open(filePath);
	if (file.fail()) STD_EXCEPT("Can't open a model file!")

	while (!file.eof())
	{
		getline(file, input);
		if (input[0] == 'v')
		{
			if (input[1] == ' ') info.vertexCount++;
			if (input[1] == 't') info.textureCount++;
			if (input[1] == 'n') info.normalCount++;
		}
		if (input[0] == 'f')
		{
			if (!checked)
			{
				int size = input.size();
				int count = 0;
				for (int i = 1; i < size; ++i) if (input[i] == ' ') count++;
				
				if (count == 3) info.isTriangulated = true;
				else info.isTriangulated = false;
				
				checked = true;
			}
			info.faceCount++;
		}
	}

	file.close();
	return info;
}

void TransformDataToDXFormat(std::string filePath, FileInfo& info, std::vector<Float3Type>& vertices,
							 std::vector<Float3Type>& texcoords, std::vector<Float3Type>& normals, std::vector<FaceType>& faces)
{
	std::ifstream file;

	vertices.resize(info.vertexCount);
	texcoords.resize(info.textureCount);
	normals.resize(info.normalCount);
	faces.resize(info.faceCount);

	file.open(filePath);

	if (file.fail()) STD_EXCEPT("Can't open a model file!")

	char input;
	int vertexIdx = 0, texIdx = 0, normalIdx = 0, faceIdx = 0;
	file.get(input);
	while (!file.eof())
	{
		// 개별 정점 데이터 읽기.
		if (input == 'v')
		{
			file.get(input);

			// 정점 위치 읽기
			if (input == ' ')
			{
				file >> vertices[vertexIdx].x >> vertices[vertexIdx].y >> vertices[vertexIdx].z;

				vertexIdx++;
			}
			// 텍스쳐 UV 좌표 읽기
			if (input == 't')
			{
				file >> texcoords[texIdx].x >> texcoords[texIdx].y;

				texIdx++;
			}
			// 노말 벡터 좌표 읽기
			if (input == 'n')
			{
				file >> normals[normalIdx].x >> normals[normalIdx].y >> normals[normalIdx].z;

				normalIdx++;
			}
		}
		// 면을 구성하는 정점들 읽기.
		if (input == 'f')
		{
			file.get(input);
			if (input == ' ')
			{
				char delim;
				
				file >> faces[faceIdx].vIndex1 >> delim >> faces[faceIdx].tIndex1 >> delim >> faces[faceIdx].nIndex1
					 >> faces[faceIdx].vIndex2 >> delim >> faces[faceIdx].tIndex2 >> delim >> faces[faceIdx].nIndex2
					 >> faces[faceIdx].vIndex3 >> delim >> faces[faceIdx].tIndex3 >> delim >> faces[faceIdx].nIndex3;
				if (!info.isTriangulated)
				{
					file >> faces[faceIdx].vIndex4 >> delim >> faces[faceIdx].tIndex4 >> delim >> faces[faceIdx].nIndex4;
				}
				faceIdx++;
			}
		}
		while (input != '\n')
		{
			file.get(input);
		}

		file.get(input);
	}

	file.close();
}

void SpawnObjInfoWindow(const FileInfo& info)
{
	if (ImGui::Begin("ObjModelInfo"))
	{
		ImGui::Text("Vertex Count : %d", info.vertexCount);
		ImGui::Text("Texture Count : %d", info.vertexCount);
		ImGui::Text("Normal Count : %d", info.vertexCount);
		ImGui::Text("Face Count : %d", info.vertexCount);
		std::string s = info.isTriangulated ? "Yes" : "No";
		ImGui::Text("Is Triangulated : %s", s);
	}
	ImGui::End();
}