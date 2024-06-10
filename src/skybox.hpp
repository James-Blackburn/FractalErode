#ifndef SKYBOX_HPP_INCLUDED
#define SKYBOX_HPP_INCLUDED

#include <vector>
#include <cstdint>

class Skybox {
private:
	unsigned int VAO;
	unsigned int textureID;
public:
	void load(std::vector<const char*>&);
	void render();
	void clean();
	inline unsigned int getTexture() const { return textureID; }

	~Skybox();
};

#endif