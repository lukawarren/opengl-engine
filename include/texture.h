#include <string>

class Texture
{
public:
    Texture(const std::string& filename);
    Texture(const Texture&) = delete;
    ~Texture();

    void bind() const;
    void unbind() const;

private:
    // OpenGL state
    unsigned int texture_id;
};