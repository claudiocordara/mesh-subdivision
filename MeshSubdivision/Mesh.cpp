#include <algorithm>
#include "Mesh.h"

void Mesh::loadOff(const char* path, bool quadToTri)
{
	cout << "Mesh initializing (to " << path << ")..\n";

	ifstream file;
	file.open(path, std::ifstream::in);

	char heading[10];

	if (file >> heading)
	{
		size_t vertSize, faceSize, dummy;
		file >> vertSize >> faceSize >> dummy;

		for (size_t i = 0; i < vertSize; i++)
		{
			Point p;
			file >> p.x >> p.y >> p.z;
			addVertex(p);
		}

		size_t sideCount = 0;
		size_t* vIds = NULL;

		if (quadToTri)
		{
			vIds = new size_t[4];
			for (size_t i = 0; i < faceSize; i++)
			{
				file >> sideCount;

				if (sideCount != 4)
				{
					cout << path << " is not a quad mesh" << endl;
					int in; cin >> in; exit(0);
				}

				file >> vIds[0] >> vIds[1] >> vIds[2] >> vIds[3];

				addFace(new size_t[3]{ vIds[0], vIds[1], vIds[2] }, 3);
				addFace(new size_t[3]{ vIds[0], vIds[2], vIds[3] }, 3);
			}
		}
		else
		{
			for (size_t i = 0; i < faceSize; i++)
			{
				file >> sideCount;

				vIds = new size_t[sideCount];
				for (size_t i = 0; i < sideCount; i++)
					file >> vIds[i];

				addFace(vIds, sideCount);
			}
		}

		cout << "Mesh has " << faces.size() << " faces, " << verts.size()
			<< " verts, " << edges.size() << " edges\nInitialization done\n";
	}
	else
	{
		cout << "cannot read " << path << endl;
	}
}

void Mesh::addFace(size_t* vertices, size_t sideCount)
{
	Face* face = new Face(faces.size());
	//TO DO: this will result in null face neighbors
	//face->faces.resize(sideCount, NULL);
	faces.push_back(face);

	for (size_t i = 0; i < sideCount; i++)
	{
		Vertex* v = verts[vertices[i]];
		Vertex* vNext = verts[vertices[i < sideCount - 1 ? i + 1 : 0]];

		face->verts.push_back(v); //sorted
		v->faces.push_back(face); //not sorted

		Edge* edge = NULL;
		int vNextPos = v->getVertPos(vNext);

		if (vNextPos == -1)
			edge = addEdge(v, vNext);
		else
		{
			edge = v->edges[vNextPos];

			face->faces.push_back(edge->faces[0]); //sorted
			//TO DO: this will result in null face neighbors
			//edge->faces[0]->faces[edge->faces[0]->getEdgePos(edge)] = face; //sorted
			edge->faces[0]->faces.push_back(face); //not sorted
		}

		face->edges.push_back(edge); //sorted
		edge->faces.push_back(face); //sorted, left face is first

		if (i > 0) edge->edges.push_back(face->edges[i - 1]); //not sorted
		else face->edges[0]->edges.push_back(edge);
	}
}

void Mesh::addVertex(const Point& p)
{
	verts.push_back(new Vertex(verts.size(), p));
}

Edge* Mesh::addEdge(Vertex * v1, Vertex * v2)
{
	v1->verts.push_back(v2); // not sorted
	v2->verts.push_back(v1); // not sorted

	Edge* edge = new Edge(edges.size(),
		Point::distance(v1->coords, v2->coords));

	edges.push_back(edge);

	v1->edges.push_back(edge); //not sorted
	v2->edges.push_back(edge); //not sorted

	edge->verts.push_back(v1);
	edge->verts.push_back(v2);

	return edge;
}

// TO DO: sort edge neighbors of an edge
void Mesh::sortCounterClockwise()
{
	for each (Vertex* v in verts)
	{
		Face* f = v->faces[0];
		Edge* edge = NULL;
		size_t eItr = 0;
		int ePos = 0;

		do
		{
			Edge* e1 = NULL;
			Edge* e2 = NULL;

			for each (Edge* e in f->edges)
				if (e->getVertPos(v) != -1)
				{
					if (e1 == NULL) e1 = e;
					else e2 = e;
				}

			edge = (f->getNextEdge(e1) == e2) ? e1 : e2;
			ePos = v->getEdgePos(edge);

			if (ePos != eItr)
			{
				swap(v->edges[ePos], v->edges[eItr]);
				swap(v->verts[ePos], v->verts[eItr++]);
			}

			swap(v->faces[ePos], v->faces[v->getFacePos(f)]);

			if (edge->faces.size() == 2)
				f = edge->faces[(f == edge->faces[0]) ? 1 : 0];
			else
				f = v->faces[0];

		} while (f != v->faces[0]);
	}
}

#define STL_HEADER_SIZE									80					///< STL header size in bytes
#define INLAB_STL_HEADER								"INLab STL file format with face normal information"

int Mesh::saveSTL(std::string _fileName, std::string _header, bool _binaryFormat) const {
	int ret = 0;
	std::ofstream strm(_fileName.c_str(), std::ios::out | std::ios::binary);
	if (strm.good() == true) {
		ret = saveSTL(strm, _header, _binaryFormat);
	}
	else {
		ret = -1;
	}
	strm.close();
	return ret;
}

int Mesh::saveSTL(std::ofstream& _file, std::string _header, bool _binaryFormat) const {
	int ret = 0;

	int cnt = 0;
	unsigned short attr_byte_count;
	attr_byte_count = 0;
	if (!_binaryFormat) {
		_file << "solid OBJECT" << std::endl;
	}
	else {
		char header[STL_HEADER_SIZE];
		memset(header, 0, STL_HEADER_SIZE);
		if (_header.size() > 0 && _header.size() < STL_HEADER_SIZE)
		{
			strcpy(header, _header.c_str());
		}
		else
		{
			strcpy(header, INLAB_STL_HEADER);
		}
		_file.write(header, STL_HEADER_SIZE);
		int triangle_count = faces.size();
		_file.write(reinterpret_cast<const char*>(&triangle_count), sizeof(int));
	}
	for (int fNdx = 0; fNdx < (int)faces.size() && ret == 0; fNdx++) {
		if (faces[fNdx]->verts.size() == 3)
		{
			Point pt1 = faces[fNdx]->verts[0]->coords;
			Point pt2 = faces[fNdx]->verts[1]->coords;
			Point pt3 = faces[fNdx]->verts[2]->coords;
			if (_binaryFormat) {
				float val = 0.0;

				val = 0.0;// normal.x();
				_file.write((char*)(&val), sizeof(val));
				val = 0.0;// normal.y();
				_file.write((char*)(&val), sizeof(val));
				val = 0.0;// normal.z();
				_file.write((char*)(&val), sizeof(val));

				val = pt1.x;
				_file.write((char*)(&val), sizeof(val));
				val = pt1.y;
				_file.write((char*)(&val), sizeof(val));
				val = pt1.z;
				_file.write((char*)(&val), sizeof(val));

				val = pt2.x;
				_file.write((char*)(&val), sizeof(val));
				val = pt2.y;
				_file.write((char*)(&val), sizeof(val));
				val = pt2.z;

				_file.write((char*)(&val), sizeof(val));
				val = pt3.x;
				_file.write((char*)(&val), sizeof(val));
				val = pt3.y;
				_file.write((char*)(&val), sizeof(val));
				val = pt3.z;
				_file.write((char*)(&val), sizeof(val));

				_file.write(reinterpret_cast<char*>(&attr_byte_count), sizeof(unsigned short));
			}
			else {
				_file << "facet normal 0.0 0.0 0.0" << std::endl;
				_file << " outer loop" << std::endl;
				_file << "  vertex " << pt1.x << " " << pt1.y << " " << pt1.z << std::endl;
				_file << "  vertex " << pt2.x << " " << pt2.y << " " << pt2.z << std::endl;
				_file << "  vertex " << pt3.x << " " << pt3.y << " " << pt3.z << std::endl;
				_file << " endloop" << std::endl;
				_file << "endfacet" << std::endl;
			}
		}
	}
	if (!_binaryFormat) {
		_file << "endsolid OBJECT" << std::endl;
	}

	return ret;
}

std::string Mesh::getNextToken(std::ifstream& _file, bool &_endLineFlag) {
	std::string ret = "";
	char c = 0;
	bool exitFlag = false;
	_endLineFlag = false;
	while (_file.good() && !_file.eof() && !exitFlag) {
		_file.get(c);
		if (c == ' ' || c == '\n') {
			if (ret.length() > 0) {
				exitFlag = true;
				if (c == '\n') {
					_endLineFlag = true;
				}
			}
		}
		else {
			if (c > '!') {
				ret.push_back(c);
			}
		}
	}
	std::transform(ret.begin(), ret.end(), ret.begin(), ::tolower);

	return ret;
}

int Mesh::loadSTL(std::string _fileName, bool _flipXY, bool _flipYZ, bool _flipXZ)
{
	int ret = 0;
	std::ifstream _file;
	bool binaryFormat = false;
	_file.open(_fileName, std::ios::binary);
	if (_file.is_open()) {
		if (_file.good() == true) {
			std::string headerStr;
			while (_file.good() && headerStr.length() < 80) {
				headerStr.push_back((char)_file.get());
			}
			std::transform(headerStr.begin(), headerStr.end(), headerStr.begin(), ::tolower);
			if (headerStr.length() == 5 && headerStr.compare("solid") == 0) {
				_file.seekg(0, std::ios::beg);
				bool exitFlag = false;
				bool endLineFlag = false;
				bool readNormalFlag = false;
				double normal[3];
				Point vertex;
				while (_file.good() && !_file.eof() && !exitFlag) {
					std::string token = getNextToken(_file, endLineFlag);
					if (token.compare("normal") == 0) {
						token = getNextToken(_file, endLineFlag);
						normal[0] = atof(token.c_str());
						token = getNextToken(_file, endLineFlag);
						normal[1] = atof(token.c_str());
						token = getNextToken(_file, endLineFlag);
						normal[2] = atof(token.c_str());
						readNormalFlag = true;
					}
					if (token.compare("vertex") == 0 && readNormalFlag) {
						token = getNextToken(_file, endLineFlag);
						vertex.x = atof(token.c_str());
						token = getNextToken(_file, endLineFlag);
						vertex.y = atof(token.c_str());
						token = getNextToken(_file, endLineFlag);
						vertex.z = atof(token.c_str());

						addVertex(vertex);
					}
					if (token.compare("endloop") == 0 && readNormalFlag) {
						size_t* faceNdx = new size_t[3];
						faceNdx[0] = (int)verts.size() - 3;
						faceNdx[0] = (int)verts.size() - 2;
						faceNdx[0] = (int)verts.size() - 1;
						addFace(faceNdx, 3);
						readNormalFlag = false;
					}
					if (token.compare("endsolid") == 0) {
						exitFlag = true;
					}
				}
			}
			else
			{
				unsigned long triangNum = 0;
				_file.read((char*)(&triangNum), sizeof(triangNum));
				if (_file.good() && triangNum > 0) {
					for (unsigned int i = 0; _file.good() && i < triangNum; i++) {
						double normal[3];
						double vertex[3][3];
						unsigned short attr = 0;

						float val = 0.0;
						for (unsigned int j = 0; _file.good() && j < 12; j++) {
							_file.read((char*)(&val), sizeof(val));
							if (j < 3) {
								normal[j] = val;
							}
							else {
								vertex[(j - 3) / 3][(j - 3) % 3] = val;
							}
						}

						Point v1(_flipYZ == true ? -vertex[0][0] : vertex[0][0], _flipXZ == true ? -vertex[0][1] : vertex[0][1], _flipXY == true ? -vertex[0][2] : vertex[0][2]);
						Point v2(_flipYZ == true ? -vertex[1][0] : vertex[1][0], _flipXZ == true ? -vertex[1][1] : vertex[1][1], _flipXY == true ? -vertex[1][2] : vertex[1][2]);
						Point v3(_flipYZ == true ? -vertex[2][0] : vertex[2][0], _flipXZ == true ? -vertex[2][1] : vertex[2][1], _flipXY == true ? -vertex[2][2] : vertex[2][2]);
						_file.read((char*)(&attr), sizeof(attr));
						size_t* faceNdx = new size_t[3];
						if (_flipXY == true) {
							faceNdx[0] = (int)verts.size();
							addVertex(v3);
							faceNdx[1] = (int)verts.size();
							addVertex(v2);
							faceNdx[2] = (int)verts.size();
							addVertex(v1);
						}
						else {
							faceNdx[0] = (int)verts.size();
							addVertex(v1);
							faceNdx[1] = (int)verts.size();
							addVertex(v2);
							faceNdx[2] = (int)verts.size();
							addVertex(v3);
						}
						addFace(faceNdx, 3);
					}
				}
			}
		}
	}
	_file.close();
	return ret;
}
