//
// Copyright (c) 2018 Amer Koleci and contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "Alimer.h"
using namespace Alimer;

namespace Alimer
{
    struct VertexColor
    {
        vec3 position;
        Color4 color;
    };

    struct VertexColorTexture
    {
        vec3 position;
        Color4 color;
        vec2 textureCoordinate;
    };

    struct PerCameraCBuffer
    {
        mat4 viewMatrix;
        mat4 projectionMatrix;
    };

    struct PerVertexData
    {
        mat4 worldMatrix;
    };

#if TODO

    class TriangleExample
    {
    public:
        ~TriangleExample() = default;

        void Initialize(GraphicsDevice* graphics)
        {
            VertexColor triangleVertices[] =
            {
                { vec3(0.0f, 0.5f, 0.0f), Color4::Red },
                { vec3(0.5f, -0.5f, 0.0f), Color4::Lime },
                { vec3(-0.5f, -0.5f, 0.0f), Color4::Blue }
            };

            std::array<VertexAttributeDescriptor, 2> vertexInputAttributs;
            // Attribute location 0: Position
            vertexInputAttributs[0].semantic = VertexElementSemantic::Position;
            vertexInputAttributs[0].format = VertexFormat::Float3;
            vertexInputAttributs[0].offset = offsetof(VertexColor, position);
            // Attribute location 1: Color
            vertexInputAttributs[1].bufferIndex = 0;
            vertexInputAttributs[1].semantic = VertexElementSemantic::Color0;
            vertexInputAttributs[1].format = VertexFormat::Float4;
            vertexInputAttributs[1].offset = offsetof(VertexColor, color);

            VertexInputFormatDescriptor inputFormatDescriptor = {};
            inputFormatDescriptor.attributes = vertexInputAttributs.data();
            inputFormatDescriptor.attributesCount = 2;
            _vertexInputFormat = graphics->CreateVertexInputFormat(&inputFormatDescriptor);

            BufferDescriptor vertexBufferDesc = {};
            vertexBufferDesc.usage = BufferUsage::Vertex;
            vertexBufferDesc.size = sizeof(triangleVertices);
            vertexBufferDesc.stride = sizeof(VertexColor);
            _vertexBuffer = graphics->CreateBuffer(&vertexBufferDesc, triangleVertices);

            // Create shader program.
            auto vertexShader = graphics->CreateShaderModule("assets://shaders/color.vert");
            auto fragmentShader = graphics->CreateShaderModule("assets://shaders/color.frag");
            _program = graphics->CreateShaderProgram(vertexShader, fragmentShader);

            BufferDescriptor uboBufferDesc = {};
            uboBufferDesc.resourceUsage = ResourceUsage::Dynamic;
            uboBufferDesc.usage = BufferUsage::Uniform;
            uboBufferDesc.size = sizeof(PerCameraCBuffer);
            _perCameraUboBuffer = graphics->CreateBuffer(&uboBufferDesc, &_camera);
        }

        void Render(CommandBuffer* context)
        {
            context->SetShaderProgram(_program.Get());
            context->SetVertexInputFormat(_vertexInputFormat.Get());
            context->BindVertexBuffer(_vertexBuffer.Get(), 0);
            context->BindBuffer(_perCameraUboBuffer.Get(), 0, 0);
            context->Draw(PrimitiveTopology::Triangles, 3);
        }

    private:
        SharedPtr<VertexInputFormat> _vertexInputFormat;
        SharedPtr<GpuBuffer> _vertexBuffer;
        SharedPtr<ShaderProgram> _program;
        SharedPtr<GpuBuffer> _perCameraUboBuffer;

        PerCameraCBuffer _camera{};
    };

    class QuadExample
    {
    public:
        void Initialize(GraphicsDevice* graphics)
        {
            VertexColor triangleVertices[] =
            {
                { vec3(-0.5f, 0.5f, 0.0f), Color4(0.0f, 0.0f, 1.0f, 1.0f) },
                { vec3(0.5f, 0.5f, 0.0f), Color4(1.0f, 0.0f, 0.0f, 1.0f) },
                { vec3(0.5f, -0.5f, 0.0f), Color4(0.0f, 1.0f, 0.0f, 1.0f) },
                { vec3(-0.5f, -0.5f, 0.0f), Color4(1.0f, 1.0f, 0.0f, 1.0f) },
            };

            BufferDescriptor vertexBufferDesc = {};
            vertexBufferDesc.usage = BufferUsage::Vertex;
            vertexBufferDesc.size = sizeof(triangleVertices);
            vertexBufferDesc.stride = sizeof(VertexColor);
            _vertexBuffer = graphics->CreateBuffer(&vertexBufferDesc, triangleVertices);

            // Create index buffer.
            const uint16_t indices[] = {
                0, 1, 2, 0, 2, 3
            };

            BufferDescriptor indexBufferDesc = {};
            indexBufferDesc.usage = BufferUsage::Index;
            indexBufferDesc.size = sizeof(indices);
            indexBufferDesc.stride = sizeof(uint16_t);
            _indexBuffer = graphics->CreateBuffer(&indexBufferDesc, indices);

            // Create shader program.
            auto vertexShader = graphics->CreateShaderModule("assets://shaders/color.vert");
            auto fragmentShader = graphics->CreateShaderModule("assets://shaders/color.frag");
            _program = graphics->CreateShaderProgram(vertexShader, fragmentShader);

            BufferDescriptor uboBufferDesc = {};
            uboBufferDesc.resourceUsage = ResourceUsage::Dynamic;
            uboBufferDesc.usage = BufferUsage::Uniform;
            uboBufferDesc.size = sizeof(PerCameraCBuffer);
            _perCameraUboBuffer = graphics->CreateBuffer(&uboBufferDesc, &_camera);
        }

        void Render(CommandBuffer* context)
        {
            context->SetShaderProgram(_program.Get());
            context->BindVertexBuffer(_vertexBuffer.Get(), 0);
            context->BindIndexBuffer(_indexBuffer.Get(), 0, IndexType::UInt16);
            context->BindBuffer(_perCameraUboBuffer.Get(), 0, 0);
            context->DrawIndexed(PrimitiveTopology::Triangles, 6);
        }

    private:
        SharedPtr<GpuBuffer> _vertexBuffer;
        SharedPtr<GpuBuffer> _indexBuffer;
        SharedPtr<ShaderProgram> _program;
        SharedPtr<GpuBuffer> _perCameraUboBuffer;

        PerCameraCBuffer _camera;
    };

    class CubeExample
    {
    public:
        void Initialize(GraphicsDevice* graphics, float aspectRatio)
        {
            _mesh = Mesh::CreateCube(graphics);

            // Uniform buffer
            //_camera.viewMatrix = Matrix4x4::CreateLookAt(vec3(0, 0, 5), vec3::zero(), vec3::unit_y());
            //_camera.projectionMatrix = Matrix4x4::CreatePerspectiveFieldOfView(M_PIDIV4, aspectRatio, 0.1f, 100);

            BufferDescriptor uboBufferDesc = {};
            uboBufferDesc.resourceUsage = ResourceUsage::Dynamic;
            uboBufferDesc.usage = BufferUsage::Uniform;
            uboBufferDesc.stride = sizeof(PerCameraCBuffer);
            uboBufferDesc.size = sizeof(PerCameraCBuffer);
            _perCameraUboBuffer = graphics->CreateBuffer(&uboBufferDesc, &_camera);

            // Per draw ubo.
            uboBufferDesc.stride = sizeof(PerVertexData);
            uboBufferDesc.size = sizeof(PerVertexData);
            _perDrawUboBuffer = graphics->CreateBuffer(&uboBufferDesc, &_perDrawData);

            // Shader program
            auto vertexShader = graphics->CreateShaderModule("assets://shaders/color.vert");
            auto fragmentShader = graphics->CreateShaderModule("assets://shaders/color.frag");
            _program = graphics->CreateShaderProgram(vertexShader, fragmentShader);
        }

        void Render(CommandBuffer* context, double deltaTime)
        {
            float time = static_cast<float>(deltaTime);
            //_perDrawData.worldMatrix = Matrix4x4::CreateRotationX(time) * Matrix4x4::CreateRotationY(time * 2) * Matrix4x4::CreateRotationZ(time * .7f);
            //_perDrawUboBuffer->SetSubData(&_perDrawData);

            // Bind shader program.
            context->SetShaderProgram(_program.Get());
            context->BindBuffer(_perCameraUboBuffer.Get(), 0, 0);
            context->BindBuffer(_perDrawUboBuffer.Get(), 0, 1);

            // Draw mesh.
            _mesh->Draw(context);
        }

    private:
        SharedPtr<Mesh> _mesh;
        SharedPtr<ShaderProgram> _program;
        SharedPtr<GpuBuffer> _perCameraUboBuffer;
        SharedPtr<GpuBuffer> _perDrawUboBuffer;

        PerCameraCBuffer _camera;
        PerVertexData _perDrawData;
    };

    class TexturedCubeExample
    {
    public:
        void Initialize(Graphics* graphics, float aspectRatio)
        {
            // A box has six faces, each one pointing in a different direction.
            const int FaceCount = 6;

            static const vec3 faceNormals[FaceCount] =
            {
                vec3(0,  0,  1),
                vec3(0,  0, -1),
                vec3(1,  0,  0),
                vec3(-1,  0,  0),
                vec3(0,  1,  0),
                vec3(0, -1,  0),
            };

            static const vec2 textureCoordinates[4] =
            {
                vec2::unit_x(),
                vec2::one(),
                vec2::unit_y(),
                vec2::zero(),
            };

            std::vector<VertexColorTexture> vertices;
            std::vector<uint16_t> indices;

            vec3 tsize(1.0f);
            tsize = tsize / 2.0f;

            // Create each face in turn.
            for (int i = 0; i < FaceCount; i++)
            {
                vec3 normal = faceNormals[i];

                // Get two vectors perpendicular both to the face normal and to each other.
                vec3 basis = (i >= 4) ? vec3::unit_z() : vec3::unit_y();

                vec3 vec3 = cross(normal, basis);
                vec3 side2 = cross(normal, side1);

                // Six indices (two triangles) per face.
                size_t vbase = vertices.size();
                indices.push_back(static_cast<uint16_t>(vbase + 0));
                indices.push_back(static_cast<uint16_t>(vbase + 1));
                indices.push_back(static_cast<uint16_t>(vbase + 2));

                indices.push_back(static_cast<uint16_t>(vbase + 0));
                indices.push_back(static_cast<uint16_t>(vbase + 2));
                indices.push_back(static_cast<uint16_t>(vbase + 3));

                // Four vertices per face.
                // (normal - side1 - side2) * tsize // normal // t0
                vertices.push_back({ Vector3::Multiply(Vector3::Subtract(Vector3::Subtract(normal, side1), side2), tsize), Color(1.0f, 0.0f, 0.0f), textureCoordinates[0] });

                // (normal - side1 + side2) * tsize // normal // t1
                vertices.push_back({ Vector3::Multiply(Vector3::Add(Vector3::Subtract(normal, side1), side2), tsize), Color(0.0f, 1.0f, 0.0f), textureCoordinates[1] });

                // (normal + side1 + side2) * tsize // normal // t2
                vertices.push_back({ Vector3::Multiply(Vector3::Add(normal, Vector3::Add(side1, side2)), tsize), Color(0.0f, 0.0f, 1.0f), textureCoordinates[2] });

                // (normal + side1 - side2) * tsize // normal // t3
                vertices.push_back({ Vector3::Multiply(Vector3::Subtract(Vector3::Add(normal, side1), side2), tsize), Color(1.0f, 0.0f, 1.0f), textureCoordinates[3] });
            }

            std::vector<VertexElement> vertexDeclaration;
            vertexDeclaration.emplace_back(VertexFormat::Float3, VertexElementSemantic::POSITION);
            vertexDeclaration.emplace_back(VertexFormat::Float4, VertexElementSemantic::COLOR);
            vertexDeclaration.emplace_back(VertexFormat::Float2, VertexElementSemantic::TEXCOORD);

            _vertexBuffer = new VertexBuffer();
            _vertexBuffer->Define(static_cast<uint32_t>(vertices.size()), vertexDeclaration, ResourceUsage::Immutable, vertices.data());

            _indexBuffer = graphics->CreateIndexBuffer(static_cast<uint32_t>(indices.size()), IndexType::UInt16, ResourceUsage::Immutable, indices.data());

            _shader = graphics->CreateShader("assets://shaders/sprite.vert", "assets://shaders/sprite.frag");

            _camera.viewMatrix = Matrix4x4::CreateLookAt(Vector3(0, 0, 5), Vector3::Zero, Vector3::UnitY);
            _camera.projectionMatrix = Matrix4x4::CreatePerspectiveFieldOfView(M_PIDIV4, aspectRatio, 0.1f, 100);
            GpuBufferDescription uboBufferDesc = {};
            uboBufferDesc.usage = BufferUsage::Uniform;
            uboBufferDesc.elementSize = sizeof(PerCameraCBuffer);
            _perCameraUboBuffer = new GpuBuffer(graphics, uboBufferDesc, &_camera);

            // Create checkerboard texture.
            ImageLevel initial = {};
            static const uint32_t checkerboard[] = {
                0xffffffffu, 0xffffffffu, 0xff000000u, 0xff000000u,
                0xffffffffu, 0xffffffffu, 0xff000000u, 0xff000000u,
                0xff000000u, 0xff000000u, 0xffffffffu, 0xffffffffu,
                0xff000000u, 0xff000000u, 0xffffffffu, 0xffffffffu,
            };
            initial.data = checkerboard;

            TextureDescriptor textureDesc = {};
            textureDesc.width = textureDesc.height = 4;
            _texture = graphics->CreateTexture(&textureDesc, &initial);
        }

        void Render(CommandContexzt* context)
        {
            context->SetShader(_shader.Get());
            context->SetVertexBuffer(0, _vertexBuffer.Get());
            context->SetIndexBuffer(_indexBuffer.Get());
            context->SetUniformBuffer(0, 0, _perCameraUboBuffer.Get());
            context->SetTexture(0, _texture.Get(), ShaderStage::Fragment);
            context->DrawIndexed(PrimitiveTopology::Triangles, 6);
        }

    private:
        SharedPtr<VertexBuffer> _vertexBuffer;
        SharedPtr<GpuBuffer> _indexBuffer;
        SharedPtr<Shader> _shader;
        SharedPtr<GpuBuffer> _perCameraUboBuffer;
        SharedPtr<Texture> _texture;

        PerCameraCBuffer _camera;
    };
#endif // TODO

    class RuntimeApplication final : public Application
    {
    public:
        RuntimeApplication();
        ~RuntimeApplication() override = default;

    private:
        void Initialize() override;
        void OnRenderFrame(CommandBuffer* commandBuffer, double frameTime, double elapsedTime) override;

    private:
        //TriangleExample _triangleExample;
        //QuadExample _quadExample;
        //CubeExample _cubeExample;
        //TexturedCubeExample _texturedCubeExample;
    };

    RuntimeApplication::RuntimeApplication()
    {
        //_settings.renderingSettings.preferredGraphicsBackend = GraphicsBackend::Vulkan;
    }

    void RuntimeApplication::Initialize()
    {
        //_triangleExample.Initialize(_graphicsDevice.Get());
        //_quadExample.Initialize(_graphicsDevice.Get());
        //_cubeExample.Initialize(_graphicsDevice.Get(), _window->GetAspectRatio());
        //_texturedCubeExample.Initialize(_graphicsDevice.Get(), _window->getAspectRatio());

        // Create triangle scene
        //auto triangleEntity = _entities.CreateEntity();
        //triangleEntity->AddComponent<TransformComponent>();
        //auto renderable = triangleEntity->AddComponent<RenderableComponent>();
        //auto mesh = new Mesh(_graphics.Get());
        //renderable->renderable = MakeDerivedHandle<Renderable, MeshRenderable>(mesh);
        //triangleEntity->AddComponent<RenderableComponent>();
    }

    void RuntimeApplication::OnRenderFrame(CommandBuffer* commandBuffer, double frameTime, double elapsedTime)
    {
        ALIMER_UNUSED(frameTime);
        ALIMER_UNUSED(elapsedTime);

        //_triangleExample.Render(commandBuffer);
        //_quadExample.Render(commandBuffer);
        //_cubeExample.Render(commandBuffer, elapsedTime);
        //_texturedCubeExample.Render(commandBuffer);
    }
}

ALIMER_APPLICATION(Alimer::RuntimeApplication);
