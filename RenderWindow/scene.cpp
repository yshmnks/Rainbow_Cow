#include "scene.h"
#include "utils.h"

using namespace Scene;
using namespace std;
using namespace glm;
/** Global variables **/
int Object::NEXTID = 0;

/* Method Definitions */
void World::addObject(Object * obj)
{
    _objects.push_back(obj);
    obj->setWorld(this);
}

void World::addObject(Camera * cam)
{
    if (_cam == nullptr)
    {
        _objects.push_back(cam);
        cam->setWorld(this);
        _cam = cam;
    }
    else
    {
        std::cout << "Cam already set!" << std::endl;
    }
}

void World::draw()
{
    for (auto &object : _objects)
    {
        auto shader = _shaderMap.find(object->getID());
        if (shader != _shaderMap.end())
        {
            object->draw(_shaderMap[object->getID()]);
        }
        else
        {
            object->draw();
        }
    }
}

void World::removeObject(Object * obj)
{
    auto sameID = [&](Object * object) { return object->getID() == obj->getID();  };
    auto to_remove = std::remove_if(std::begin(_objects), std::end(_objects), sameID);
    _objects.erase(to_remove);
}

void World::assignShader(Object * obj, Shader * shader)
{
    _shaderMap[obj->getID()] = shader;
}
Shader * World::findShader(Object * obj)
{
    return _shaderMap[obj->getID()];
}


void Object::draw()
{
    if (!_visible) return;

    glPushMatrix();
    glTranslated(_tx, _ty, _tz);
    glRotated(_psi, 0, 0, 1);
    glRotated(_the, 0, 1, 0);
    glRotated(_phi, 0, 0, 1);

    doDraw();

    glPopMatrix();
}

void Object::draw(Shader * shader)
{
    if (!_visible) return;

    glPushMatrix();
    glTranslated(_tx, _ty, _tz);
    glRotated(_psi, 0, 0, 1);
    glRotated(_the, 0, 1, 0);
    glRotated(_phi, 0, 0, 1);

    shader->link();
    doDraw();
    shader->unlink();
    glPopMatrix();
}


void Camera::doDraw()
{
}

void Grid::doDraw()
{
    for (int r = -(_rows / 2); r <= (_rows / 2); r++)
    {
        GlutDraw::drawLine(-(_cols / 2.0f)*_gap, 0, r*_gap,
            (_cols / 2.0f)*_gap, 0, r*_gap);
    }
    for (int c = -(_cols / 2); c <= (_cols / 2); c++)
    {
        GlutDraw::drawLine(c*_gap, 0, -(_rows / 2.0f)*_gap,
            c*_gap, 0, (_rows / 2.0f)*_gap);
    }
}

void Arrow::doDraw() {
    glPushAttrib(GL_CURRENT_BIT);
    glColor3f(_color[0], _color[1], _color[2]);
    GlutDraw::drawLine(_tail[0], _tail[1], _tail[2], _head[0], _head[1], _head[2]);

    //float d = distance(_head, _tail);
    //vec3 n = normalize(_head - _tail);
    //vec3 pivot = _head - (d / 10)*n;
    //glBegin(GL_TRIANGLE_FAN);
    //glVertex3f(_head[0], _head[1], _head[2]);
    //glEnd();
    glPopAttrib();
}

void Sphere::doDraw()
{
    GlutDraw::drawSphere(_r, _n, _m);
}

void ObjGeometry::doDraw()
{
    if (!_geomReady)
    {
        _readGeom();
    }
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, &_vertices[0]);
    glNormalPointer(GL_FLOAT, 0, &_normals[0]);

    glDrawArrays(GL_TRIANGLES, 0, _vertices.size());

    return;
}

// Adopted from http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/
int ObjGeometry::_readGeom()
{
    std::vector< int > vertexIndices, uvIndices, normalIndices;
    std::vector< glm::vec3 > tempVertices;
    std::vector< glm::vec2 > tempUVs;
    std::vector< glm::vec3 > tempNormals;
    int lineCount = 0;
    int faceCount = 0;
    int vertCount = 0;

    std::ifstream file;
    file.open(_filename, std::ios::in);
    if (!file.is_open())
    {
        std::cout << "Could not open " << _filename << std::endl;
        return -1;
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream linestream(line);
        std::string type;
        if (line.find("v ") == 0)
        {
            glm::vec3 vertex;
            linestream >> type >> vertex.x >> vertex.y >> vertex.z;
            vertex.x = vertex.x;
            vertex.y = vertex.y;
            vertex.z = vertex.z;
            tempVertices.push_back(vertex);
            vertCount++;
        }
        else if (line.find("vn ") == 0)
        {
            glm::vec3 normal;
            linestream >> type >> normal.x >> normal.y >> normal.z;
            tempNormals.push_back(normal);
        }
        else if (line.find("vt ") == 0)
        {
            glm::vec2 uv;
            linestream >> type >> uv.x >> uv.y;
            tempUVs.push_back(uv);
        }
        else if (line.find("f ") == 0)
        {
            int vertexIndex[3], normalIndex[3], uvIndex[3];
            char delim;
            linestream >> type >>
                vertexIndex[0] >> delim >> uvIndex[0] >> delim >> normalIndex[0] >>
                vertexIndex[1] >> delim >> uvIndex[1] >> delim >> normalIndex[1] >>
                vertexIndex[2] >> delim >> uvIndex[2] >> delim >> normalIndex[2];

            for (int i = 0; i < 3; i++)
            {
                vertexIndices.push_back(vertexIndex[i]);
                normalIndices.push_back(normalIndex[i]);
                uvIndices.push_back(uvIndex[i]);
            }
            faceCount++;
        }

        lineCount++;
        //if (lineCount % 1000 == 0)
        //{
        //    std::cout << "Parsing obj line: " << lineCount << "\r";
        //}
    }
    std::cout << "Parsed " << lineCount << " lines Verts: " << vertCount << " Triangles: " << faceCount << std::endl;
    file.close();

    for (int i = 0; i < vertexIndices.size(); i++)
    {
        int vertexIndex = vertexIndices[i];
        glm::vec3 vertex = tempVertices[vertexIndex - 1];
        _vertices.push_back(vertex);
    }
    for (int i = 0; i < normalIndices.size(); i++)
    {
        int normalIndex = normalIndices[i];
        glm::vec3 normal = tempNormals[normalIndex - 1];
        _normals.push_back(normal);
    }
    for (int i = 0; i < uvIndices.size(); i++)
    {
        int uvIndex = uvIndices[i];
        glm::vec2 uv = tempUVs[uvIndex - 1];
        _uvs.push_back(uv);
    }

    _geomReady = true;

    return lineCount;
}

World & Scene::createWorld()
{
    World * new_world = new World();
    return *new_world;
}

void Shader::_initShaders()
{
    if (_vertfile == "" || _fragfile == "")
    {
        std::cout << "No shaders! Initialization failing." << std::endl;
        return;
    }
    else if (_shaderReady)
    {
        std::cout << "Shader has already initialized." << std::endl;
        return;
    }

    char *vs, *fs;

    if (_vertfile == "" && _fragfile == ""){ return; }
    _program = glCreateProgram();

    if (_vertfile != "")
    {
        _vertex = glCreateShader(GL_VERTEX_SHADER);
        vs = textFileRead(_vertfile.c_str());
        const char * vv = vs;
        glShaderSource(_vertex, 1, &vv, NULL);
        free(vs);
        glCompileShader(_vertex);
        if (_checkShaderError(_vertex))
        {
            std::cout << _vertfile << " compiled successfully." << std::endl;
            glAttachShader(_program, _vertex);
        }
    }
    if (_fragfile != "")
    {
        _frag = glCreateShader(GL_FRAGMENT_SHADER);
        fs = textFileRead(_fragfile.c_str());
        const char * ff = fs;
        glShaderSource(_frag, 1, &ff, NULL);
        free(fs);
        glCompileShader(_frag);
        if (_checkShaderError(_frag))
        {
            std::cout << _fragfile << " compiled successfully." << std::endl;
            glAttachShader(_program, _frag);
        }
    }

    glLinkProgram(_program);

    glDetachShader(_program, _vertex);
    glDetachShader(_program, _frag);
    glDeleteShader(_vertex);
    glDeleteShader(_frag);

    _shaderReady = true;
    return;
}

bool Shader::_checkShaderError(GLuint shader)
{
    GLint result = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

    if (result == GL_TRUE) return true;

    GLint logsize = 0;
    char * log;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logsize);
    log = (char *)malloc(logsize + 1);
    glGetShaderInfoLog(shader, logsize, &result, log);

    std::cout << log << std::endl;
    return false;
}

void Shader::link()
{
    glUseProgram(_program);
}

void Shader::unlink()
{
    glUseProgram(0);
}









std::vector<float> parseLine(const std::string& line, const char& c) {
    std::vector<int> sp;
    sp.push_back(-1);
    int spNext = line.find(c, sp.back() + 1);
    if (spNext!=std::string::npos) sp.push_back(spNext);
    while (spNext != std::string::npos) {
        spNext = line.find(c, sp.back() + 1);
        sp.push_back(spNext);
    }
    if (sp.size() == 1) return std::vector<float>(0);
    std::vector<float> nums;
    for (int i = 0; i < sp.size() - 1; i++){
        std::string s = line.substr(sp[i] + 1, sp[i + 1]);
        nums.push_back(atof(s.c_str()));
    }
    return nums;
}
bool intersect_union(const set<int>& fSet0, const set<int>& fSet1, set<int>& fIntersect, set<int>& fUnion){
    fIntersect.clear();
    fUnion = fSet1;
    for (set<int>::iterator f0 = fSet0.begin(); f0 != fSet0.end(); f0++){
        bool uniqueFlag = true;
        for (set<int>::iterator f1 = fSet1.begin(); f1 != fSet1.end(); f1++){
            if (*f0 == *f1) {
                fIntersect.insert(*f1);
                uniqueFlag = false;
                break;
            }
        }
        if (uniqueFlag == true) fUnion.insert(*f0);
    }
    if (fIntersect.size() == 0) return false;
    else return true;
}
void MeshObject::makeProgressiveMeshFile() {
    ofstream oFile;
    oFile.open(_oFileName);
    oFile << "OFFPM\n";
    oFile << nVertices() << ' ' << _faces.size() << '\n';
    oFile << _adjacency.size() << ' ' << nVisibleFaces() << ' ' << _nCollapses << '\n';
    oFile << _xMin << ' ' << _xMax << ' ' << _yMin << ' ' << _yMax << ' ' << _zMin << ' ' << _zMax << '\n';
    // write vertices to string
    for (map<int, set<int>>::iterator adj = _adjacency.begin(); adj != _adjacency.end(); adj++) {
        int vIndex = adj->first;
        set<int> fIndexSet = adj->second;
        oFile << vIndex << ' ';
        oFile << _vertexPositions[vIndex][0] << ' ' << _vertexPositions[vIndex][1] << ' ' << _vertexPositions[vIndex][2] << ' ';
        oFile << _vertexNormals[vIndex][0] << ' ' << _vertexNormals[vIndex][1] << ' ' << _vertexNormals[vIndex][2] << '\n';
    }
    // write faces to string
    vector<int> visFaceIndices = visibleFaces();
    for (int i = 0; i < visFaceIndices.size(); i++){
        int f = visFaceIndices[i];
        vector<int> v = _faces[f];
        oFile << f << ' ' << v[0] << ' ' << v[1] << ' ' << v[2] << '\n';
    }
    // write collapses to string
    for (int i = 0; i < _v0.size(); i++) {
        oFile << _v0[i] << ' ';
        oFile << _xyz0[i][0] << ' ' << _xyz0[i][1] << ' ' << _xyz0[i][2] << ' ';
        oFile << _n0[i][0] << ' ' << _n0[i][1] << ' ' << _n0[i][2] << ' ';
        oFile << _xyz[i][0] << ' ' << _xyz[i][1] << ' ' << _xyz[i][2] << ' ';
        oFile << _n[i][0] << ' ' << _n[i][1] << ' ' << _n[i][2];
        for (int j = 0; j < _fVec[i].size(); j++) {
            oFile << ' ' << _fVec[i][j];
        }
        oFile << '\n'; //////////////////////////////////////////////////////////
        oFile << _v1[i] << ' ';
        oFile << _xyz1[i][0] << ' ' << _xyz1[i][1] << ' ' << _xyz1[i][2] << ' ';
        oFile << _n1[i][0] << ' ' << _n1[i][1] << ' ' << _n1[i][2];
        for (int j = 0; j < _fVec1[i].size(); j++) {
            oFile << ' ' << _fVec1[i][j];
        }
        oFile << '\n'; //////////////////////////////////////////////////////////
        for (int j = 0; j < _fVecR[i].size(); j++) {
            oFile << _fVecR[i][j] << ' ' << _fVecRijk[i][j][0] << ' ' << _fVecRijk[i][j][1] << ' ' << _fVecRijk[i][j][2];
            if (j < _fVecR[i].size() - 1) oFile << ' ';
        }
        oFile << '\n'; //////////////////////////////////////////////////////////
    }
    oFile.close();
}


void MeshObject::readGeom() {
    string file = _iFileName;
    string line;
    ifstream modelfile(_iFileName);
    if (!modelfile.is_open()) exit;
    getline(modelfile, line);
    if (line == "OFF") {
        _drawVertexNormals = true;
        _format = "off";
        readGeomOFF();
    }
    else if (line == "OFFPM") {
        _drawVertexNormals = true;
        _format = "offpm";
        readGeomOFFPM();
    }
    else printf("ERROR: Mesh File Type Unrecognized\n");
}
void MeshObject::readGeomOFF(){
    printf("------------------------- READING .OFF FILE -------------------------\n");
    string file = _iFileName;
    string line;
    ifstream modelfile(_iFileName);
    if (!modelfile.is_open()) exit;
    getline(modelfile, line);
    if (line != "OFF") exit;
    getline(modelfile, line);
    vector<float> pl = parseLine(line, ' ');
    int nV = pl[0];
    int nF = pl[1];
    _dummy.resize(nV);
    int printStepV = ceil((float)nV / 100.0);
    int printStepF = ceil((float)nF / 100.0);
    _complexity = nV;
    _nCollapses = 0;
    _lastUpdate.resize(nV, _nCollapses);
    _pairs.reserve(3 * nF); // for a closed mesh we give twice the leeway since _pairs includes out-of-date pairs
    _partners.resize(nV);
    _quadrics.resize(nV);
    _vertexPositions.resize(2 * nV, vec3(0, 0, 0));
    _vertexNormals.resize(2 * nV, vec3(0, 0, 0));
    _vertexColors.resize(2 * nV, vec4(0, 0, 0, 0));
    _faces.resize(nF, { 0, 0, 0 });
    _triangleIndices.resize(3 * nF, 0);
    _lineIndices.resize(2 * nF, 0);
    _faceNormals.resize(nF, vec3(0, 0, 0));
    _faceAreas.resize(nF, 0);
    _xMin = 0;
    _xMax = 0;
    _yMin = 0;
    _yMax = 0;
    _zMin = 0;
    _zMax = 0;
    for (int i = 0; i < nV; i++){
        if (i%printStepV == 0) printf("We're on vertex %i/%i\r", i + 1, nV);
        getline(modelfile, line);
        pl = parseLine(line, ' ');
        float x = pl[0];
        float y = pl[1];
        float z = pl[2];
        if (i == 0) { _xMin = x; _xMax = x; _yMin = y; _yMax = y; _zMin = z; _zMax = z; }
        else {
            if (x < _xMin) _xMin = x;
            if (y < _yMin) _yMin = y;
            if (z < _zMin) _zMin = z;
            if (x > _xMax) _xMax = x;
            if (y > _yMax) _yMax = y;
            if (z > _zMax) _zMax = z;
        }
        _lineIndices[2 * i + 0] = i;
        _lineIndices[2 * i + 1] = nV + i;
        _vertexPositions[i] = vec3(x, y, z);
        _vertexColors[i] = vec4(1, 1, 1, 1);
    }
    printf("We're on vertex %i/%i\n", nV, nV);
    float dAvg = 0; // rough estimate for average edge length. Not actually correct, but it suffices for picking appropriate _t
    for (int i = 0; i < nF; i++){
        if (i%printStepF == 0) printf("We're on face %i/%i\r", i + 1, nF);
        getline(modelfile, line);
        pl = parseLine(line, ' ');
        int v0 = pl[1];
        int v1 = pl[2];
        int v2 = pl[3];
        _faces[i] = { v0, v1, v2 };
        _triangleIndices[3 * i + 0] = v0;
        _triangleIndices[3 * i + 1] = v1;
        _triangleIndices[3 * i + 2] = v2;
        _adjacency[v0].insert(i);
        _adjacency[v1].insert(i);
        _adjacency[v2].insert(i);
        float d = glm::distance(_vertexPositions[v0], _vertexPositions[v1]);
        d += glm::distance(_vertexPositions[v1], _vertexPositions[v2]);
        d += glm::distance(_vertexPositions[v2], _vertexPositions[v0]);
        d /= 3 * nF;
        dAvg += d;
    }
    printf("We're on face %i/%i\n", nF, nF);
    printf("PROCESSING: Vertex/Face Normals\n");
    reComputeVertexNormals();
    printf("            Vertex Quadrics\n");
    reComputeQuadrics();
    printf("            Quadric Error Metrics...\n");
    _t = -1;
    setT(0.0 * dAvg);
    printf("---------------------------------------------------------------------\n");
    _geomReady = true;
    /*for (int i = 0; i < nVertices(); i++) {
        _vertexPositions[i + nVertices()] = _vertexPositions[i] + dAvg*_vertexNormals[i];
    }*/

}
void MeshObject::readGeomOFFPM() {
    int lineNumber = 0;
    printf("------------------------- READING .OFFPM FILE -------------------------\n");
    string file = _iFileName;
    string line;
    ifstream modelfile(_iFileName);
    if (!modelfile.is_open()) exit;
    getline(modelfile, line);
    lineNumber++;
    if (line != "OFFPM") exit;
    getline(modelfile, line);
    lineNumber++;
    vector<float> pl = parseLine(line, ' ');
    int nV_full = pl[0];
    int nF_full = pl[1];
    _dummy.resize(nV_full);
    printf("Reserving space for up to %i vertices and %i faces\n", nV_full, nF_full);
    getline(modelfile, line);
    lineNumber++;
    pl = parseLine(line, ' ');
    int nV = pl[0];
    int nF = pl[1];
    int nC = pl[2];
    int _nVcollapsed = pl[0];
    int _nFcollapsed = pl[1];
    int printStepV = ceil((float)nV / 100);
    int printStepF = ceil((float)nF / 100);
    int printStepC = ceil((float)nC / 100);
    getline(modelfile, line);
    pl = parseLine(line, ' ');
    _xMin = pl[0];
    _xMax = pl[1];
    _yMin = pl[2];
    _yMax = pl[3];
    _zMin = pl[4];
    _zMax = pl[5];
    _complexity = nV;
    _dummyCollapsed.resize(nV);
    _vertexPositions.resize(nV_full, vec3(0, 0, 0));
    _vertexNormals.resize(nV_full, vec3(0, 0, 0));
    _vertexColors.resize(nV_full, vec4(1, 1, 1, 1));
    _faces.resize(nF_full, { 0, 0, 0 });
    //_faceAreas.resize(nF_full, 0);
    //_faceNormals.resize(nF_full, { 0, 0, 0 });
    _triangleIndices.resize(3 * nF_full, 0);
    float xMin = 0;
    float xMax = 0;
    float yMin = 0;
    float yMax = 0;
    float zMin = 0;
    float zMax = 0;
    for (int i = 0; i < nV; i++){
        if (i%printStepV == 0) printf("We're on vertex %i/%i\r", i + 1, nV);
        getline(modelfile, line);
        lineNumber++;
        pl = parseLine(line, ' ');
        int v = pl[0];
        float x = pl[1];
        float y = pl[2];
        float z = pl[3];
        float nx = pl[4];
        float ny = pl[5];
        float nz = pl[6];
        if (i == 0) { xMin = x; xMax = x; yMin = y; yMax = y; zMin = z; zMax = z; }
        else {
            if (x < xMin) xMin = x;
            if (y < yMin) yMin = y;
            if (z < zMin) zMin = z;
            if (x > xMax) xMax = x;
            if (y > yMax) yMax = y;
            if (z > zMax) zMax = z;
        }
        _vertexPositions[v] = vec3(x, y, z);
        if (nx == INFINITY || ny == INFINITY || nz == INFINITY || nx == -INFINITY || ny == -INFINITY || nz == -INFINITY) _vertexNormals[v] = vec3(0, 0, 0);
        else _vertexNormals[v] = vec3(nx, ny, nz);
    }
    printf("We're on vertex %i/%i\n", nV, nV);
    for (int i = 0; i < nF; i++){
        if (i%printStepF == 0) printf("We're on face %i/%i\r", i + 1, nF);
        getline(modelfile, line);
        lineNumber++;
        pl = parseLine(line, ' ');
        int f = pl[0];
        int v0 = pl[1];
        int v1 = pl[2];
        int v2 = pl[3];
        _faces[f] = { v0, v1, v2 };
        _triangleIndices[3 * f + 0] = v0;
        _triangleIndices[3 * f + 1] = v1;
        _triangleIndices[3 * f + 2] = v2;
    }
    printf("We're on face %i/%i\r", nF, nF);
    _v0.reserve(nC);
    _v1.reserve(nC);
    _n0.reserve(nC);
    _n1.reserve(nC);
    _n.reserve(nC);
    _xyz0.reserve(nC);
    _xyz1.reserve(nC);
    _xyz.reserve(nC);
    _fVec.reserve(nC);
    _fVec1.reserve(nC);
    _fVecR.reserve(nC);
    _fVecRijk.reserve(nC);
    vector<int> f;
    cout << endl;
    for (int i = 0; i < nC; i++){
        if (i%printStepC == 0) printf("We're on collapse %i/%i\r", i + 1, nC);
        getline(modelfile, line); /////
        lineNumber++;
        pl = parseLine(line, ' ');
        _v0.push_back(pl[0]);
        _xyz0.push_back(vec3(pl[1], pl[2], pl[3]));
        if (pl[4] == INFINITY || pl[5] == INFINITY || pl[6] == INFINITY || pl[4] == -INFINITY || pl[5] == -INFINITY || pl[6] == -INFINITY) _n0.push_back(vec3(0, 0, 0));
        else _n0.push_back(vec3(pl[4], pl[5], pl[6]));
        _xyz.push_back(vec3(pl[7], pl[8], pl[9]));
        if (pl[10] == INFINITY || pl[11] == INFINITY || pl[12] == -INFINITY || pl[10] == -INFINITY || pl[11] == -INFINITY || pl[12] == INFINITY) _n.push_back(vec3(0, 0, 0));
        else _n.push_back(vec3(pl[10], pl[11], pl[12]));
        f.clear();
        for (int j = 13; j < pl.size(); j++) f.push_back(pl[j]);
        _fVec.push_back(f);
        getline(modelfile, line); /////
        lineNumber++;
        pl = parseLine(line, ' ');
        _v1.push_back(pl[0]);
        _xyz1.push_back(vec3(pl[1], pl[2], pl[3]));
        if (pl[4] == INFINITY || pl[5] == INFINITY || pl[6] == INFINITY || pl[4] == -INFINITY || pl[5] == -INFINITY || pl[6] == -INFINITY) _n1.push_back(vec3(0, 0, 0));
        else _n1.push_back(vec3(pl[4], pl[5], pl[6]));
        f.clear();
        for (int j = 7; j < pl.size(); j++) f.push_back(pl[j]);
        _fVec1.push_back(f);
        getline(modelfile, line); /////
        lineNumber++;
        pl = parseLine(line, ' ');
        f.clear();
        vector<vector<int>> ijk;
        for (int j = 0; j < pl.size(); j += 4){
            f.push_back(pl[j + 0]);
            ijk.push_back({ (int)pl[j + 1], (int)pl[j + 2], (int)pl[j + 3] });
        }
        _fVecR.push_back(f);
        _fVecRijk.push_back(ijk);
    }
    printf("We're on collapse %i/%i\n", nC, nC);
    printf("-----------------------------------------------------------------------\n");
    _geomReady = true;
}
pair<int, int> MeshObject::randomEdge() {
    if (_adjacency.size() == 0) return pair<int, int>({ -1, -1 });
    if (_adjacency.size() == 1) return pair<int, int>({ -1, -1 });
    if (_adjacency.size() == 2) return pair<int, int>(_adjacency.begin()->first, _adjacency.rbegin()->first);
    //if (_adjacency.size() < 2) {
    //    printf("WARNING: No more pairs left to collapse,\n");
    //    return pair<int,int>({ _adjacency.begin()->first, _adjacency.begin()->first });
    //}
    int v0, v1;
    int tryCount = 0;
    while (true) {
        if (tryCount > 1000) break;
        tryCount++;
        int m = fmin(_adjacency.size() - 1, (float)_adjacency.size()*rand() / RAND_MAX);
        map<int, set<int>>::iterator mIt = _adjacency.begin();
        for (int i = 0; i < m; i++) mIt++;
        v0 = mIt->first;
        set<int> fSet0 = mIt->second;
        if (fSet0.size() == 0) {
            //printf("Vertex %i at (%f, %f, %f) has no adjacent faces. Trying again.\n", v0, _vertexPositions[v0][0], _vertexPositions[v0][1], _vertexPositions[v0][2]);
            continue;
        }
        int f = fmin(fSet0.size() - 1, (float)fSet0.size()*rand() / RAND_MAX);
        set<int>::iterator fIt = fSet0.begin();
        int r = 1 + fmin(1, 2.0*rand() / RAND_MAX);
        for (int i = 0; i < f; i++) fIt++;
        for (int i = 0; i < 3; i++) {
            if (_faces[*fIt][i] == v0) {
                v1 = _faces[*fIt][(i + r) % 3];
                break;
            }
        }
        return pair<int, int>(fmin(v0, v1), fmax(v0, v1));
    }
    // random edge could not be found. resorting to deterministic search
    for (map<int, set<int>>::iterator it = _adjacency.begin(); it != _adjacency.end(); it++) {
        if (it->second.size() > 0) return pair<int, int>(it->first, *it->second.begin());
    } // if we reach this point there really aren't any edges left, so we just return the dummy pair {-1,-1}
    return pair<int, int>({ -1, -1 });
}


void MeshObject::collapse(const int& v0, const int& v1) { collapse(v0, v1, _approximationMethod); }
void MeshObject::collapse(const int& v0, const int& v1, const int& approximationMethod) { // the former vertex is kept. the latter is discarded from adjacency
    if (v0 == -1 && v1 == -1) {
        printf("WARNING: No edges remain.\n");
        return;
    }
    if (_adjacency.size() < 2) {
        printf("WARNING: No more pairs to collapse.\n");
        return;
    }
    if (_adjacency.size() < 4) {
        for (map<int, set<int>>::iterator it = _adjacency.begin(); it != _adjacency.end(); it++) {
            _lineIndices[2 * it->first + 0] = 0;
            _lineIndices[2 * it->first + 1] = 0;
        }
    }
    int adjInSize = _adjacency.size();
    map<int, set<int>> adjIn = _adjacency;
    _nCollapses++;
    _lastUpdate[v0] = _nCollapses;
    _lastUpdate[v1] = _nCollapses;
    _v0.push_back(v0);
    _v1.push_back(v1);
    _xyz0.push_back(_vertexPositions[v0]);
    _xyz1.push_back(_vertexPositions[v1]);
    _n0.push_back(_vertexNormals[v0]);
    _n1.push_back(_vertexNormals[v1]);
    _lineIndices[2 * v1 + 0] = 0;
    _lineIndices[2 * v1 + 1] = 0;
    _vertexPositions[v0] = mergedCoordinates(v0, v1, approximationMethod);
    _xyz.push_back(_vertexPositions[v0]);
    set<int> fSet0 = _adjacency[v0];
    set<int> fSet1 = _adjacency[v1];
    set<int> fUnion, fIntersect; // shared faces along edge (typically two unless mesh isn't "closed")
    set<int> fDis0 = fSet0;
    set<int> fDis1 = fSet1;

    fUnion = fSet1;
    for (set<int>::iterator f0 = fSet0.begin(); f0 != fSet0.end(); f0++){
        bool uniqueFlag = true;
        for (set<int>::iterator f1 = fSet1.begin(); f1 != fSet1.end(); f1++){
            if (*f0 == *f1) {
                fIntersect.insert(*f1);
                fDis0.erase(*f1);
                fDis1.erase(*f1);
                uniqueFlag = false;
                break;
            }
        }
        if (uniqueFlag == true) fUnion.insert(*f0);
    }
    _fVec1.push_back(vector<int>(fDis1.begin(), fDis1.end()));
    vector<int> fShared = vector<int>(fIntersect.begin(), fIntersect.end());
    _fVecR.push_back(fShared);
    vector<vector<int>> ijk;
    for (int i = 0; i < fShared.size(); i++) ijk.push_back(_faces[fShared[i]]);
    _fVecRijk.push_back(ijk);

    /*printf("collapsing %i %i\n", v0, v1);
    Sphere* sphere = new Sphere(0.01,20,20);
    sphere->setTx(_xyz.back()[0] + (_xMin + _xMax) / 2);
    sphere->setTy(_xyz.back()[1] + (_yMin + _yMax) / 2);
    sphere->setTz(_xyz.back()[2] + (_zMin + _zMax) / 2);
    _world->addObject(sphere);*/
    _partners[v0].erase(v1);
    _partners[v1].erase(v0);
    set<int> vSet1;
    for (set<int>::iterator f = fSet1.begin(); f != fSet1.end(); f++) {
        for (int i = 0; i < 3; i++) {
            if (_faces[*f][i] == v0 || _faces[*f][i] == v1) continue;
            vSet1.insert(_faces[*f][i]);
        }
    }
    for (set<int>::iterator v = vSet1.begin(); v != vSet1.end(); v++) {
        _partners[*v].erase(v1);
        _partners[*v].insert(v0);
        _partners[v0].insert(*v);
    }


    /*for (int i = 0; i < nVertices(); i++) {
        printf("\n%i: ", i);
        for (set<int>::iterator it = _partners[i].begin(); it != _partners[i].end(); it++){
            printf("%i ", *it);
        }
    }
    printf("\n");*/


    vector<int> vFinVec; // the third vertices (!=v0 && !=v1) of the shared faces
    for (set<int>::iterator f = fIntersect.begin(); f != fIntersect.end(); f++) { // For each of the shared faces
        _triangleIndices[3 * (*f) + 0] = 0; // Make the shared face degenerate in the index buffer so it doesn't get drawn
        _triangleIndices[3 * (*f) + 1] = 0;
        _triangleIndices[3 * (*f) + 2] = 0;
        for (int corner = 0; corner < 3; corner++) { // For each vertex that is connected to the shared face _faces[*f][v] ...
            set<int>::iterator it = _adjacency[_faces[*f][corner]].find(*f); // Remove the shared face *f from that vertex's list of adjacent faces
            if (it != _adjacency[_faces[*f][corner]].end()) _adjacency[_faces[*f][corner]].erase(it);
            if (_faces[*f][corner] != v0 &&_faces[*f][corner] != v1) vFinVec.push_back(_faces[*f][corner]);
        }
    }

    // change all associations of faces adjacent to v1 from "v1 to v0"
    fSet1 = _adjacency[v1]; // this is important. we don't want to change _faces[fIntersect], since we need it later for writing to ProgMesh file
    for (set<int>::iterator f = fSet1.begin(); f != fSet1.end(); f++) {
        for (int corner = 0; corner < 3; corner++) {
            if (_faces[*f][corner] != v1) continue;
            _faces[*f][corner] = v0;
            _triangleIndices[3 * (*f) + corner] = v0;
            _adjacency[v0].insert(*f); // DON'T FORGET TO ADD V1's NEIGHBORS TO V0's ADJACENCY
        }
    }
    _adjacency.erase(v1); // Remove v1 from the _adjacency list
    // Update normals for FACES adjacent to v0
    fSet0 = _adjacency[v0];
    set<int> vSet0;
    for (set<int>::iterator f = fSet0.begin(); f != fSet0.end(); f++) {
        vec3 p[3] = { _vertexPositions[_faces[*f][0]], _vertexPositions[_faces[*f][1]], _vertexPositions[_faces[*f][2]] };
        for (int c = 0; c < 3; c++) vSet0.insert(_faces[*f][c]);
        vec3 n = cross(p[1] - p[0], p[2] - p[0]);
        float nLength = glm::distance(vec3(0, 0, 0), n);
        _faceAreas[*f] = nLength / 2.0f;
        if (nLength>0) _faceNormals[*f] = n / nLength;
    }
    // Update normals for VERTICES adjacent to above faces (including v0 itself)
    for (set<int>::iterator v = vSet0.begin(); v != vSet0.end(); v++) {
        vec3 n(0, 0, 0);
        float nScale = 0;
        for (set<int>::iterator fs = _adjacency[*v].begin(); fs != _adjacency[*v].end(); fs++) {
            n += _faceNormals[*fs];
            nScale += _faceAreas[*fs];
        }
        n = normalize(n / (float)_adjacency[*v].size());
        nScale = sqrt(nScale / (float)_adjacency[*v].size());
        _vertexNormals[*v] = n;
        _vertexNormals[*v + nVertices()] = n;
        _vertexPositions[*v + nVertices()] = _vertexPositions[*v] + nScale*n;
    }
    _n.push_back(_vertexNormals[v0]);
    // Update the Quadric and Metric Priority Queue
    updateQuadricsAndMetrics(v0, v1, set<int>(vFinVec.begin(), vFinVec.end()));
    // save the collapse to File (important that this comes BEFORE fin removal, since it is a recursive call)
    _fVec.push_back(vector<int>(_adjacency[v0].begin(), _adjacency[v0].end()));
    // FINALLY! remove the fins if any exist ---------------------------------------------
    if (_allowFins == false) {
        for (int i = 0; i < vFinVec.size(); i++) {
            fSet0 = _adjacency[v0];
            int vFin = vFinVec[i];
            int uFin = -1; // the third vertex index of the fin
            int fFin = -1; // the face index of the fin
            bool finFound = false;
            for (set<int>::iterator f = fSet0.begin(); f != fSet0.end(); f++) {
                Face corners = _faces[*f];
                for (int j = 0; j < 3; j++) {
                    if (corners[j] != vFin) continue;
                    int u = 0;
                    if (corners[(j + 1) % 3] == v0) u = corners[(j + 2) % 3];
                    else u = corners[(j + 1) % 3];
                    if (u != uFin) {
                        uFin = u;
                        fFin = *f;
                    }
                    else {
                        //printf(" FIN ");
                        finFound = true;
                        collapse(v0, vFin, BINARY_APPROXIMATION_METHOD); // to remove fin call collapseEdge(_,_) recursively
                    }
                    break;
                }
            }
            if (finFound == true) continue;
        }
    }

    /*if (_adjacency.size() != nVertices() - _nCollapses) {
        printf("IMPOSSIBRU!!!!!! %i %i merged on collapse %i\n", v0, v1, _nCollapses);
        for (map<int, set<int>>::iterator it = adjIn.begin(); it != adjIn.end(); it++){
            if (it->first == v1) continue;
            map<int, set<int>>::iterator it2 = _adjacency.find(it->first);
            if (it2 == _adjacency.end()) {
                printf("  vertex %i has gone missing\n", it->first);
                printf("  initial adjacency for missing vertex %i:", it->first);
                for (set<int>::iterator it3 = adjIn[it->first].begin(); it3 != adjIn[it->first].end(); it3++) {
                    printf("  %i", *it3);
                }
                printf("\n");
                printf("  initial adjacency for vertex %i:", v0);
                for (set<int>::iterator it3 = adjIn[v0].begin(); it3 != adjIn[v0].end(); it3++) {
                    printf("  %i", *it3);
                }
                printf("\n");
                printf("  initial adjacency for vertex %i:", v1);
                for (set<int>::iterator it3 = adjIn[v1].begin(); it3 != adjIn[v1].end(); it3++) {
                    printf("  %i", *it3);
                }
                printf("\n");
            }
        }
        system("PAUSE");
    }*/
}

void MeshObject::collapseRandomEdge(const int& approximationMethod) {
    pair<int, int> re = randomEdge();
    collapse(re.first, re.second, approximationMethod);
}

void MeshObject::setT(const float& t) {
    printf("Setting distance threshold to %f\n", t);
    printf("  Updating quadric error metrics between sufficiently close vertices\n");
    if (t == _t) return;
    vector<Edge> pairVec;
    pairVec.reserve(3 * _faces.size() / 2); // reserving more crashes
    if (t == 0) {
        int counter = 0;
        _pairs = reservable_priority_queue<Edge>();
        for (int i = 0; i < _faces.size(); i++) {
            vector<int> f = _faces[i];
            for (int j = 0; j < 3; j++) {
                int u0 = fmin(f[j], f[(j + 1) % 3]);
                int u1 = fmax(f[j], f[(j + 1) % 3]);
                set<int>::iterator it = _partners[u0].find(u1);
                if (it != _partners[u0].end()) continue; // already exists
                _partners[u0].insert(u1);
                _partners[u1].insert(u0);
                pairVec.push_back(Edge(u0, u1, metric(u0, u1), _nCollapses, _nCollapses));
            }
            counter++;
            if (counter % 100 == 0) printf("  %i\r", counter);
        }
    }
    else if (t > _t) {
        int counter = 0;
        _pairs = reservable_priority_queue<Edge>();
        for (map<int, set<int>>::iterator vt = _adjacency.begin(); vt != _adjacency.end(); vt++) {
            int v = vt->first;
            for (map<int, set<int>>::iterator ut = _adjacency.begin(); ut != vt; ut++) {
                int u = ut->first;
                bool check;
                if (t > 0) check = isEdge(u, v) || glm::distance(_vertexPositions[u], _vertexPositions[v]) < t;
                else check = isEdge(u, v);
                if (check) {
                    pairVec.push_back(Edge(u, v, metric(u, v), _nCollapses, _nCollapses));
                    _partners[u].insert(v);
                    _partners[v].insert(u);
                    counter++;
                    if (counter % 100 == 0) printf("  %i\r", counter);
                }
            }
        }
    }
    else { // t < _t
        while (!_pairs.empty()) {
            Edge e = _pairs.top();
            _pairs.pop();
            if (glm::distance(_vertexPositions[e._u0], _vertexPositions[e._u0]) > t) {
                _partners[e._u0].erase(e._u1);
                _partners[e._u1].erase(e._u0);
                continue;
            }
            pairVec.push_back(e);
        }
    }
    priority_queue<Edge> pairs = priority_queue<Edge>(pairVec.begin(), pairVec.end());
    swap(_pairs, pairs);
    printf("  %i collapseable vertex pairs found\n", _pairs.size());
    _t = t;
    /*for (int i = 0; i < nVertices(); i++) {
        printf("\n%i: ", i);
        for (set<int>::iterator it = _partners[i].begin(); it != _partners[i].end(); it++){
            printf("%i ", *it);
        }
    }
    printf("\n"); //*/
}
bool MeshObject::isEdge(const int& v0, const int& v1) {
    set<int> adjFaces = _adjacency[v0];
    if (adjFaces.size() == 0) return false;
    for (set<int>::iterator f = adjFaces.begin(); f != adjFaces.end(); f++) {
        for (int c = 0; c < 3; c++) {
            if (_faces[*f][c] == v1) return true;
        }
    }
    return false;
}
bool MeshObject::atCorner(const int& v) {
    if (_adjacency[v].size() == 1) return true;
    else return false;
}
bool MeshObject::atBoundary(const int& v) {
    vector<int> cs;
    for (set<int>::iterator fAdj = _adjacency[v].begin(); fAdj != _adjacency[v].end(); fAdj++) {
        vector<int> f = _faces[*fAdj];
        for (int i = 0; i < 3; i++) {
            if (f[i] == v) continue;
            cs.push_back(f[i]);
        }
    }
    sort(cs.begin(), cs.end());
    for (int i = 0; i < cs.size(); i += 2) {
        if (cs[i] != cs[i + 1]) return true;
    }
    return false;
}
float MeshObject::faceArea(const int& f) {
    vec3 p0 = _vertexPositions[_faces[f][0]];
    vec3 p1 = _vertexPositions[_faces[f][1]];
    vec3 p2 = _vertexPositions[_faces[f][2]];
    vec3 e01 = p1 - p0; // edge 0->1 of face
    vec3 e02 = p2 - p0; //      0->2
    return glm::distance(vec3(0,0,0),cross(e01, e02));
}
vec3 MeshObject::faceNormal(const int& f) {
    vec3 p0 = _vertexPositions[_faces[f][0]];
    vec3 p1 = _vertexPositions[_faces[f][1]];
    vec3 p2 = _vertexPositions[_faces[f][2]];
    vec3 e01 = p1 - p0; // edge 0->1 of face
    vec3 e02 = p2 - p0; //      0->2
    return normalize(cross(e01, e02));
}
void MeshObject::reComputeFaceNormals() {
    for (int f = 0; f < _faces.size(); f++) {
        vec3 p0 = _vertexPositions[_faces[f][0]];
        vec3 p1 = _vertexPositions[_faces[f][1]];
        vec3 p2 = _vertexPositions[_faces[f][2]];
        vec3 e01 = p1 - p0; // edge 0->1 of face
        vec3 e02 = p2 - p0; //      0->2
        vec3 n = cross(e01, e02);
        float nLength = glm::distance(vec3(0, 0, 0), n);// .length();
        _faceAreas[f] = nLength / 2.0f;
        if (nLength>0) _faceNormals[f] = n / nLength;
    }
    _faceNormalsReady = true;
}
void MeshObject::reComputeVertexNormals() {
    if (_faceNormalsReady == false) reComputeFaceNormals();
    for (map<int, set<int>>::const_iterator i = _adjacency.begin(); i != _adjacency.end(); i++) {
        vec3 n(0, 0, 0);
        float nScale = 0;
        set<int> adjFaces = i->second; // adjacent faces
        for (set<int>::iterator j = adjFaces.begin(); j != adjFaces.end(); j++){
            n += _faceNormals[*j];
            nScale += _faceAreas[*j];
        }
        n = normalize(n / (float)adjFaces.size());
        nScale = sqrt(nScale / (float)adjFaces.size());
        _vertexNormals[i->first] = n;
        _vertexNormals[i->first + nVertices()] = n;
        _vertexPositions[i->first + nVertices()] = _vertexPositions[i->first] + nScale*n;
    }
}
vec3 MeshObject::mergedCoordinates(const int& v0, const int& v1, const int& approximationMethod) {
    if (approximationMethod == BINARY_APPROXIMATION_METHOD) return _vertexPositions[v0];
    if (approximationMethod == MIDPOINT_APPROXIMATION_METHOD) return (_vertexPositions[v0] + _vertexPositions[v1]) / 2.0f;
    if (approximationMethod == QUADRIC_APPROXIMATION_METHOD) {
        mat4 dQ = _quadrics[v0] + _quadrics[v1];
        dQ[0][3] = 0;
        dQ[1][3] = 0;
        dQ[2][3] = 0;
        dQ[3][3] = 1;
        vec4 optPos = inverse(dQ)*vec4(0, 0, 0, 1);
        return vec3(optPos[0], optPos[1], optPos[2]) / optPos[3];
    }
}
void MeshObject::reComputeQuadrics() {
    for (map<int, set<int>>::iterator m = _adjacency.begin(); m != _adjacency.end(); m++) {
        _quadrics[m->first] = quadric(m->first);
    }
    _quadricsReady = true;
}
mat4 MeshObject::quadric(const int& v) {
    if (_faceNormalsReady == false) reComputeFaceNormals();
    mat4 Q(0.0f);
    for (set<int>::iterator f = _adjacency[v].begin(); f != _adjacency[v].end(); f++){
        vec3 n = _faceNormals[*f];
        float d = -dot(_vertexPositions[_faces[*f][0]], n);
        vec4 plane = vec4(n[0], n[1], n[2], d);
        Q += outerProduct(plane, plane);
    }
    return Q;
}
pair<vec3, float> MeshObject::metric(const int& v0, const int& v1) {
    if (_quadricsReady == false) reComputeQuadrics();
    mat4 Q = _quadrics[v0] + _quadrics[v1];
    mat4 dQ = Q;
    dQ[0][3] = 0;
    dQ[1][3] = 0;
    dQ[2][3] = 0;
    dQ[3][3] = 1;
    if (fabs(determinant(dQ)) < 0.000001) return pair<vec3, float>(vec3(INFINITY, INFINITY, INFINITY), INFINITY);
    vec4 vPrime = inverse(dQ)*vec4(0, 0, 0, 1);
    vec4 Q_vPrime = Q*vPrime;
    vec3 vPrimeInhomo(vPrime[0] / vPrime[3], vPrime[1] / vPrime[3], vPrime[2] / vPrime[3]);
    return pair<vec3, float>(vPrimeInhomo, dot(Q_vPrime, Q_vPrime));
}
void MeshObject::updateQuadricsAndMetrics(const int& v0, const int& v1, const set<int>&vShared) {
    if (_pairs.size() < 2) {
        _pairs = reservable_priority_queue<Edge>();
        return;
    }
    _quadrics[v0] = _quadrics[v0] + _quadrics[v1];
    set<int> fSet = _adjacency[v0];
    set<int> vSet;
    for (set<int>::iterator f = fSet.begin(); f != fSet.end(); f++) {
        for (int i = 0; i < 3; i++) vSet.insert(_faces[*f][i]);
    }
    for (set<int>::iterator v = vSet.begin(); v != vSet.end(); v++) {
        _quadrics[*v] = quadric(*v);
        _lastUpdate[*v] = _lastUpdate[v0];
    }
    //////////////////////////////////////////////////////////////////
    set<Edge> edgeSet;
    for (set<int>::iterator v = vSet.begin(); v != vSet.end(); v++) {
        //printf("%i  ", _partners[*v].size());
        for (set<int>::iterator p = _partners[*v].begin(); p != _partners[*v].end(); p++) {
            int x = fmin(*v, *p);
            int y = fmax(*v, *p);
            edgeSet.insert(Edge(x, y, metric(x, y), _lastUpdate[x], _lastUpdate[y]));
        }
    }
    //printf("\nedgeSet size: %i\n", edgeSet.size());
    for (set<Edge>::iterator it = edgeSet.begin(); it != edgeSet.end(); it++) {
        _pairs.push(*it);
    }
}

int MeshObject::nVisibleFaces() {
    int count = 0;
    for (int i = 0; i < _faces.size(); i++) {
        if (_triangleIndices[3 * i + 0] == _triangleIndices[3 * i + 1]) continue;
        if (_triangleIndices[3 * i + 1] == _triangleIndices[3 * i + 2]) continue;
        if (_triangleIndices[3 * i + 2] == _triangleIndices[3 * i + 0]) continue;
        count++;
    }
    return count;
}
vector<int> MeshObject::visibleFaces() {
    vector<int> visFaces;
    visFaces.reserve(_faces.size());
    for (int i = 0; i < _faces.size(); i++) {
        if (_triangleIndices[3 * i + 0] == _triangleIndices[3 * i + 1]) continue;
        if (_triangleIndices[3 * i + 1] == _triangleIndices[3 * i + 2]) continue;
        if (_triangleIndices[3 * i + 2] == _triangleIndices[3 * i + 0]) continue;
        visFaces.push_back(i);
    }
    return visFaces;
}

float MeshObject::avgEdgeLength() { // approximate cause i don't feel like dealing with the double counting at the borders
    vector<int> f = visibleFaces();
    int n = f.size();
    float dAvg = 0;
    for (int i = 0; i < n; i++) {
        vector<int> c = _faces[f[i]]; // face corner vertex indices
        float d = 0;
        for (int j = 0; j < 3; j++) d += glm::distance(_vertexPositions[c[j]], _vertexPositions[c[(j + 1) % 3]]);
        dAvg += d / (3 * n);
    }
    return dAvg;
}

void MeshObject::quadricSimplify() {
    if (_pairs.size() < 1) {
        //printf("No more pairs to collapse.\n");
        return;
    }
    Edge e = _pairs.top();
    _pairs.pop();
    //printf("%i %i %i %i\n", e._u0, e._u1, e._c0, e._c1);
    while (e._c0 != _lastUpdate[e._u0] || e._c1 != _lastUpdate[e._u1]) {
        if (_pairs.size() < 1) {
            //printf("No more pairs to collapse.\n");
            return;
        }
        e = _pairs.top();
        _pairs.pop();
        //printf("  %i %i %i %i\n", e._u0, e._u1, e._c0, e._c1);
    }
    if (e._qem < INFINITY) collapse(e._u0, e._u1, QUADRIC_APPROXIMATION_METHOD);
    else collapse(e._u0, e._u1, MIDPOINT_APPROXIMATION_METHOD);
}

void MeshObject::doDraw()
{
    if (!_geomReady) readGeom();
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    if (_customColors==true) glEnableClientState(GL_COLOR_ARRAY);
    else glDisableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    /*pair<vector<vec3>, vector<vec4>> asdf = vRedundant();
    vector<vec3> v = asdf.first;
    vector<vec4> c = asdf.second;
    glVertexPointer(3, GL_FLOAT,0, &v[0]);
    glColorPointer(4, GL_FLOAT, 0, &c[0]);
    glDrawArrays(GL_TRIANGLES, 0, v.size());
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);//*/


    glVertexPointer(3, GL_FLOAT, 0, &_vertexPositions[0]);
    glNormalPointer(GL_FLOAT, 0, &_vertexNormals[0]);
    glColorPointer(4, GL_FLOAT, 0, &_vertexColors[0]);
    GLuint IBO;
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*_triangleIndices.size(), &_triangleIndices[0], GL_STATIC_DRAW);
    glDrawElements(GL_TRIANGLES, _triangleIndices.size(), GL_UNSIGNED_INT, 0);
    if (_drawMode == 0) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if (_drawVertexNormals == false) return;
    ////////////////////////
    ///// DRAW NORMALS /////
    ////////////////////////
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*_lineIndices.size(), &_lineIndices[0], GL_STATIC_DRAW);

    glDrawElements(GL_LINES, _lineIndices.size(), GL_UNSIGNED_INT, 0);//*/

    return;
}




void MeshObject::collapseTo(const float& newComplexity) {
    float fOldCollapseIndex = (float)(nVerticesCollapsed() + _v0.size()) - _complexity; // this corresponds to the current mesh "adjacency" (BEFORE carrying out collapse[index])
    float fNewCollapseIndex = (float)(nVerticesCollapsed() + _v0.size()) - newComplexity; // these are both the index OF THE COLLAPSE
    //float fOldCollapseIndex = (float)_nV - _complexity;
    //float fNewCollapseIndex = (float)_nV - newComplexity;
    //float fOldCollapseIndex = (float)_vertexPositions.size() - _complexity;
    //float fNewCollapseIndex = (float)_vertexPositions.size() - newComplexity;
    fOldCollapseIndex = fmin(fmax(0, fOldCollapseIndex), _v0.size());
    fNewCollapseIndex = fmin(fmax(0, fNewCollapseIndex), _v0.size());
    int oldCollapseIndex = fOldCollapseIndex;
    int newCollapseIndex = fNewCollapseIndex;
    float alpha = fNewCollapseIndex - newCollapseIndex;
    //printf("current collapse index: %i\n", oldCollapseIndex);
    //printf("    new collapse index: %i\n", newCollapseIndex);
    if (_complexity == newComplexity) {
        printf("no changes to make\n");
        return;
    }
    else if (_complexity > newComplexity) { // COLLAPSE
        for (int i = oldCollapseIndex; i < newCollapseIndex; i++) { // for each "full" collapse to get to newPos
            _vertexPositions[_v0[i]] = _xyz[i]; // update the coordinates of v=v0
            _vertexNormals[_v0[i]] = _n[i];
            for (int j = 0; j < _fVec[i].size(); j++) { // for each face in the updated adjacency for v=v0
                int f = _fVec[i][j];
                for (int k = 0; k < 3; k++) {
                    if (_faces[f][k] == _v1[i]) _faces[f][k] = _v0[i];
                    if (_triangleIndices[3 * f + k] == _v1[i]) _triangleIndices[3 * f + k] = _v0[i]; // change all corners from v1 to v=v0
                }
            }
            for (int j = 0; j < _fVecR[i].size(); j++) { // for each face that is shared between v0,v1
                int f = _fVecR[i][j];
                for (int k = 0; k < 3; k++) _triangleIndices[3 * f + k] = 0; // obliterate it from existence
            }
        }
    }
    else if (_complexity < newComplexity) { // SPLIT
        for (int i = oldCollapseIndex; i > newCollapseIndex-1; i--) {
            if (i == _v0.size()) continue;
            _vertexPositions[_v0[i]] = _xyz0[i];
            _vertexPositions[_v1[i]] = _xyz1[i];
            _vertexNormals[_v0[i]] = _n0[i];
            _vertexNormals[_v1[i]] = _n1[i];
            for (int j = 0; j < _fVecR[i].size(); j++) {
                int f = _fVecR[i][j];
                _faces[f] = _fVecRijk[i][j];
                for (int k = 0; k < 3; k++) _triangleIndices[3 * f + k] = _faces[f][k];
            }
            for (int j = 0; j < _fVec1[i].size(); j++) {
                int f = _fVec1[i][j];
                for (int k = 0; k < 3; k++) {
                    if (_faces[f][k] == _v0[i]) _faces[f][k] = _v1[i];
                    _triangleIndices[3 * f + k] = _faces[f][k];
                }
            }
        }
    }
    _complexity = fmin(fmax(_vertexPositions.size() - _v0.size(), newComplexity), _vertexPositions.size()); // update the current _complexity
    // GEOMORPH: small alpha means we are close to the full split
    if (newCollapseIndex >= _v0.size()) return;
    _vertexPositions[_v0[newCollapseIndex]] = (1.0f - alpha)*_xyz0[newCollapseIndex] + alpha*_xyz[newCollapseIndex];
    _vertexPositions[_v1[newCollapseIndex]] = (1.0f - alpha)*_xyz1[newCollapseIndex] + alpha*_xyz[newCollapseIndex];
    _vertexNormals[_v0[newCollapseIndex]] = (1.0f - alpha)*_n0[newCollapseIndex] + alpha*_n[newCollapseIndex];
    _vertexNormals[_v1[newCollapseIndex]] = (1.0f - alpha)*_n1[newCollapseIndex] + alpha*_n[newCollapseIndex];
}

void MeshObject::makeAdjacencyFromIndices() {
    vector<int> vv = visibleFaces();
    for (int i = 0; i < vv.size(); i++) {
        vector<int> f = _faces[vv[i]];
        for (int j = 0; j < 3; j++) {
            _adjacency[f[j]].insert(vv[i]);
        }
    }
}