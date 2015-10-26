#ifndef OBJLOADER_H
#define OBJLOADER_H

bool loadOBJ(
	std::string objFileName,
	std::vector<glm::vec3> & out_vertices, 
	std::vector<glm::vec2> & out_uvs 
);

bool loadOBJ_wo_tex(
	const char * path,
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec3> & out_normals
	);

bool loadOBJ_wo_tex_norm(
	std::string objFileName,
	std::vector<glm::vec3> & out_vertices
	);



bool loadAssImp(
	const char * path, 
	std::vector<unsigned short> & indices,
	std::vector<glm::vec3> & vertices,
	std::vector<glm::vec2> & uvs,
	std::vector<glm::vec3> & normals
);

#endif