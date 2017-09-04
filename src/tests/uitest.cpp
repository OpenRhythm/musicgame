// Copyright (c) 2015-2017 Matthew Sitton <matthewsitton@gmail.com>
// See LICENSE in the project root for license information.

#include "config.hpp"

#include <iostream>
#include <vector>
#include <memory>
#include <iterator>
#include <string>
#include <ios>
#include <stdexcept>
#include <array>

#include <SDL.h>

#include "window.hpp"
#include "context.hpp"
#include "events.hpp"
#include "timing.hpp"
#include "renderer/shader.hpp"
#include "renderer/renderer.hpp"
#include "renderer/texture.hpp"
#include "filesystem.hpp"

//Build an AST for representation of expressions for UI layout. This will simplify the layout code dramatically, as we wont need to hardcode the supported operations.

enum RelativeMode
{
    standard,
    center,
    inverse
};

struct Rect2D
{
    // The transform origin of this object. 
    RelativeMode originX;
    RelativeMode originY;

    // The transformation from the parent.
    float x;
    float y;

    // The point on the parent we are relative to
    RelativeMode parentRelationX;
    RelativeMode parentRelationY;

    float width; // The total width of the Rect
    float height; // The total height of the Rect

};

// This structure will part of the output of the layout code.
// If we used all element slots the whole computed layout would be 1kb.
// This is the data which will be used to render the UI each frame.

// The actual layout code can be ran on a seperate thread since this
// is what is actually used to render and wont be changed during layout.
// instead a new set of RectAbs structures will be generated each time the layout is
// recalculated.
struct RectAbs
{

    // The transformation from the parent.
    float x;
    float y;

    float width; // The total width of the Rect
    float height; // The total height of the Rect

};


// This puts our max number of ui elements at 65,536 which seams completely reasonable.
// This also puts the whole tree structure at 2kb which is def small enough to fit into the cpu cache.
// This puts 2 nodes per cache line, which should be good.
using TreeNodeID = uint16_t;

// This children max was chosen so that each LayoutNode would fit fully in a cache line.
const int tree_children_max = 14;

// These sizes would give us 4681 root level objects each with 14 sub objects or
// 334 object each with 2 levels deep of 14 objects. Or 23 top level objects with each
// object having 3-levels deep tree structure each with 14 children. Totally overkill.

// Along with the rects which will have the same number as nodes
// This brings the total size of the entire ui layout system to 4kb.

// A node is only for defining the layout of the tree.
struct LayoutNode
{
    TreeNodeID id;
    TreeNodeID parentId;

    // This is the number of direct children not including any children they may have.
    std::array<TreeNodeID, tree_children_max> children;
};

// any elements at the root level can be parallelized 

// TODO - Make this a template for container type?
class LayoutTree
{
public:
    LayoutTree();
    void add_rect(Rect2D &rect, LayoutNode *parent);
private:
    std::vector<Rect2D> m_rects;
    std::vector<LayoutNode> m_nodes;
};

void rect_position(Rect2D &parent, Rect2D &child,
    RelativeMode parentRelationX, RelativeMode parentRelationY)
{
        float positionX = parent.x;
        float positionY = parent.y;
        int posSignX = 1;
        int posSignY = 1;

        // calculate position from selected parent point.
        switch(parentRelationX)
        {
            case standard: break;
            case center:
                positionX += parent.width/2.0;
                break;
            case inverse:
                positionX += parent.width;
                posSignX = -1;
                break;
            default: break;
        }

        switch(parentRelationY)
        {
            case standard: break;
            case center:
                positionY += parent.height/2.0;
                break;
            case inverse:
                positionY += parent.height;
                posSignY = -1;
                break;
            default: break;
        }

        // calculate child transform origin.
        switch(child.originX)
        {
            case standard:
                positionX += child.x * posSignX;
            break;
            case center:
                positionX += (child.x - (child.width/2.0)) * posSignX;
                break;
            case inverse:
                positionX += (child.x - parent.width) * posSignX;
                break;
            default: break;
        }
        switch(child.originY)
        {
            case standard:
                positionY += child.y * posSignY;
            break;
            case center:
                positionY += (child.y - (child.height/2.0)) * posSignY;
                break;
            case inverse:
                positionY += (child.y - parent.height) * posSignY;
                break;
            default: break;
        }
}

class UiTest
{
public:
    UiTest()
    :m_width(1920),
    m_height(1080),
    m_fullscreen(false),
    m_title("OpenRhythm"),
    m_context(3, 2, 8),
    m_window(m_width, m_height, m_fullscreen, m_title),
    m_eventManager(),
    m_eventPump(&m_eventManager)
    {

        m_running = true;


        m_window.make_current(&m_context);

        m_window.disable_sync();

        if (!gladLoadGL())
        {
            throw std::runtime_error(_("Error: GLAD failed to load."));
        }

        m_lis.handler = std::bind(&UiTest::event_handler, this, std::placeholders::_1);
        m_lis.mask = ORCore::EventType::EventAll;

        m_eventManager.add_listener(m_lis);

        m_fpsTime = 0.0;

        m_ss = std::cout.precision();
        m_renderer.init_gl();

        ORCore::ShaderInfo vertInfo {GL_VERTEX_SHADER, "./data/shaders/main.vs"};
        ORCore::ShaderInfo fragInfo {GL_FRAGMENT_SHADER, "./data/shaders/main.fs"};

        m_program = m_renderer.add_program(ORCore::Shader(vertInfo), ORCore::Shader(fragInfo));

        m_texture = m_renderer.add_texture(ORCore::loadSTB("data/icon.png"));
        resize(m_width, m_height);

        ORCore::RenderObject obj;

        // Background Neck
        obj.set_program(m_program);

        obj.set_primitive_type(ORCore::Primitive::triangle);
        obj.set_texture(m_texture);

        obj.set_scale(glm::vec3{16.0f, 16.0f, 0.0f});
        obj.set_translation(glm::vec3{0.0f, 0.0f, 0.0f});
        obj.set_geometry(ORCore::create_rect_mesh(glm::vec4{1.0f,1.0f,1.0f,1.0f}));
        m_mouseObj = m_renderer.add_object(obj);

        m_renderer.commit();

        GLint  iMultiSample = 0;
        GLint  iNumSamples = 0;
        glGetIntegerv(GL_SAMPLE_BUFFERS, &iMultiSample);
        glGetIntegerv(GL_SAMPLES, &iNumSamples);

        glClearColor(0.5, 0.5, 0.5, 1.0);
    }

    ~UiTest()
    {
        m_window.make_current(nullptr);
    }


    void start()
    {
        while (m_running)
        {
            m_fpsTime += m_clock.tick();
            m_eventPump.process();

            update();
            render();

            m_renderer.check_error();

            m_window.flip();
            if (m_fpsTime >= 2.0) {
                std::cout.precision (5);
                std::cout << "FPS: " << m_clock.get_fps() << std::endl;
                std::cout.precision(m_ss);
                m_fpsTime = 0;
            }
        }
    }


    void resize(int width, int height)
    {
        m_width = width;
        m_height = height;
        glViewport(0, 0, m_width, m_height);
        m_ortho = glm::ortho(0.0f, static_cast<float>(m_width), static_cast<float>(m_height), 0.0f, -1.0f, 2.0f);
        m_renderer.set_camera_transform("ortho", m_ortho);
    }

    bool event_handler(const ORCore::Event &event)
    {
        switch (event.type)
        {
            case ORCore::Quit:
            {
                m_running = false;
                break;
            }
            case ORCore::MouseMove:
            {
                auto ev = ORCore::event_cast<ORCore::MouseMoveEvent>(event);
                m_mouseX = ev.x;
                m_mouseY = ev.y;
                break;
            }
            case ORCore::WindowSize:
            {
                auto ev = ORCore::event_cast<ORCore::WindowSizeEvent>(event);
                resize(ev.width, ev.height);
                break;
            }
            case ORCore::KeyDown: {
                auto ev = ORCore::event_cast<ORCore::KeyDownEvent>(event);
                switch(ev.key)
                {
                    case ORCore::KeyCode::KEY_F:
                        std::cout << "Key F" << std::endl;
                        break;
                    default:
                        std::cout << "Other Key" << std::endl;
                        break;
                }
            }
            default:
                break;
        }
        return true;
    }

    void update()
    {
        // Static aways in view objects to be transformed with the camera
        auto obj = m_renderer.get_object(m_mouseObj);
        obj->set_translation(glm::vec3(m_mouseX, m_mouseY, 0.0f));
        m_renderer.update_object(m_mouseObj);

        m_renderer.commit();
    }

    void render()
    {
        glClear(GL_COLOR_BUFFER_BIT);
        m_renderer.render();
    }
private:
    bool m_running;
    double m_fpsTime;
    int m_width;
    int m_height;
    bool m_fullscreen;
    std::string m_title;

    int m_mouseX = 0;
    int m_mouseY = 0;

    ORCore::FpsTimer m_clock;

    ORCore::Context m_context;
    ORCore::Window m_window;
    ORCore::EventManager m_eventManager;
    ORCore::EventPumpSDL2 m_eventPump;
    ORCore::Renderer m_renderer;
    ORCore::Listener m_lis;

    ORCore::ObjectID m_mouseObj;

    ORCore::TextureID m_texture;
    ORCore::ProgramID m_program;

    std::streamsize m_ss;
    glm::mat4 m_ortho;
};


// Eventually we will want to load configuration files somewhere in here.
int main(int argc, char** argv)
{

    try
    {
        UiTest game;
        game.start();
    }
    catch (std::runtime_error &err)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, _("Runtime Error"), err.what(), nullptr);
        return 1;
    }
    return 0;
}
