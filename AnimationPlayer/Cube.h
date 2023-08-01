#pragma once
#include "GameObject.h"

class Cube : public GameObject
{
public :
			Cube(glm::vec3 position, glm::vec3 halfSideLength, glm::vec3 color = glm::vec3(0.9f, 0.9f, 0.9f));
			~Cube(void);
	
	void	update(Shader& shader, float deltaTime) override;

private :
	GLuint	_vbo;
	GLuint	_vao;
	GLuint	_ebo;
};
