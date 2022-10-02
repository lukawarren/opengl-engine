#include <vector>
#include <cstddef>

class Mesh
{
public:
    Mesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices);
    ~Mesh();

    void make_vao(const unsigned int attribute, const unsigned int format,
        const unsigned int dimensions, const std::vector<float>& data);

    void bind() const;
    void unbind() const;
    void draw() const;

private:
    // OpenGL state
    unsigned int vao;
    unsigned int ebo;
    std::vector<unsigned int> vbos;

    // Mesh info
    size_t indices;
};