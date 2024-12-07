#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>

#include <GLFW/glfw3.h>
#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#undef GLAD_GL_IMPLEMENTATION
#include <glm/glm.hpp>

#include "camera.h"
#include "opengl_context.h"
#include "utils.h"

#define ANGLE_TO_RADIAN(x) (float)((x)*M_PI / 180.0f)
#define RADIAN_TO_ANGLE(x) (float)((x)*180.0f / M_PI)

#define CIRCLE_SEGMENT 64
#define SECTOR 36
#define STACK 18

#define ROTATE_SPEED 1.0f
#define MOVING_SPEED ROTATE_SPEED / 20.f
#define BALL_MOVING_SPEED 0.05f
#define SWING_SPEED 2.0f

#define SWING_SPEED_BACK 0.5f

#define ROLLING_SPEED 5.0f

#define HEIGHT_SPEED 0.025f
#define HEIGHT_UP_LIM 1.5f
#define HEIGHT_DOWN_LIM 0.0f

#define CONSTRAIN_ANGLE -45

#define SILVER 0.75, 0.75, 0.75
#define CYAN 0, 1, 1
#define PURPLE 1, 0, 1

#define TOLERATE 0.75f
#define DISTANCE_TOLERANCE 3
enum class angle { NONE = 0, CLOCKWISE = -1, COUNTERCLOCKWISE = 1 };
enum class scalar { NONE = 0, MINUS = -1, PLUS = 1 };

// to check the putter is swing or not
bool isSwing = false;
// to check the ball is hit or not
bool isHit = false;

// the scalar for the putter translation
scalar delta_xzpos = scalar::NONE;
// the scalar for the ball translation
scalar delta_ballpos = scalar::NONE;
// the scalar for the ball rotation
angle delta_ball_rotate = angle::NONE;
// the scalar for the putter swing
angle delta_x_rotate = angle::NONE;
// the scalar for the putter yaw
angle delta_y_rotate = angle::NONE;

// the angle for the ball rotation
float ball_rotate = 0.0f;
// the angle for the putter swing
float x_rotate = 0.0f;
// the angle for the yaw rotation
float y_rotate = 0.0f;

// the position of the putter
glm::vec3 xzpos(0.0f, 0.0f, 0.0f);
// the forward vector for putter
glm::vec3 forward_vector(0.0f, 0.0f, 1.0f);
// the forward vector for the ball translation
glm::vec3 ball_forward(0.0f, 0.0f, 1.0f);
// x,y,z coordinate for ball rotation
glm::vec3 ball_rotate_normal(0, 1, 0);
// the position of the ball
glm::vec3 ballpos(2.0f, 0.25f, 2.0f);
// the position of the ball when it is hit at the start
glm::vec3 startpos(0, 0, 0);

// all the former rotations for the ball
glm::mat4 currentRotation = glm::identity<glm::mat4>();



// =================================================== my variable ===================================================
// global
void print_vector(glm::vec3 vec) {
    // print out a vector for debugging
    printf("(%f, %f, %f)\n", vec.x, vec.y, vec.z);
}
enum class swing_status { NONE = 0, DOWN = -1, UP = 1, BACK = 2 }; // define the putter swing status

// ball
glm::vec3 ball_center_position(2.0f, 0.25f, 2.0f); // the position of the center of the golf ball
bool is_hit = false; // whether the ball is hit
bool is_rolling = false; // whether the ball is rolling
std::vector<std::pair<float, glm::vec3> > ball_all_rotation; // all ball rolling direction and angle
glm::vec3 last_ball_center_position(2.0f, 0.25f, 2.0f); // the ball center position before rolling

//putter
float height_set = 0.3f; //for height control

glm::vec3 putter_center_position(0.0f, (0.0f + height_set), 0.0f); // the position of the center of the putter (the intercetion of hitting part and rod)
float swing_angle = 0.0f; // the swing angle of the putter

float swing_add_constraint = 0.0f; //the swing angle additional constraint based on the back swing angle

swing_status putter_swing_status = swing_status::NONE; // the swing status of the putter

// hitting part
float hitting_part_xz_plane_rotate_angle = 0.0f; // the yawing angle of the hitting part
glm::vec3 hitting_part_center_position(-0.5f, (0.4f + height_set), 0.0f); // the position of the center of the hitting part

// rod
glm::vec3 rod_center_position(0.0f, (3.0f + height_set), 0.0f); // the position of the center of the rod

// key status
bool up_pressed = false;
bool down_pressed = false;
bool right_pressed = false;
bool left_pressed = false;

bool space_pressed = false;
bool space_release = false;

bool upward_pressed = false;
bool downward_pressed = false;
// ===================================================================================================================


void resizeCallback(GLFWwindow* window, int width, int height) {
    OpenGLContext::framebufferResizeCallback(window, width, height);
    auto ptr = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (ptr) {
        ptr->updateProjectionMatrix(OpenGLContext::getAspectRatio());
    }
}

void keyCallback(GLFWwindow* window, int key, int, int action, int) {
    // There are three actions: press, release, hold(repeat)
    if (action == GLFW_REPEAT) return;

    // Press ESC to close the window.
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        return;
    }
    /* TODO#4-1: Detect key-events, perform all the controls for the golfing
     *       1. Use switch && case to find the key you want.
     *       2. Define and modify some global variable to trigger update in rendering loop
     * Hint:
     *       glfw3's key list (https://www.glfw.org/docs/3.3/group__keys.html)
     *       glfw3's action codes (https://www.glfw.org/docs/3.3/group__input.html#gada11d965c4da13090ad336e030e4d11f)
     * Note:
     *       You should finish rendering your putter and golfball first.
     *       Otherwise you will spend a lot of time debugging this with a black screen.
     */

     /*
       in order to have the putter can continue move while the corresponding key is under pressed, we use some bool
       variable to record whether the key is still being pressed (up_pressed, down_pressed, ...)
     */

    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_RIGHT: {
            std::cout << "right pressed" << std::endl;
            std::cout << hitting_part_xz_plane_rotate_angle << std::endl;
            right_pressed = true;
            break;
        }

        case GLFW_KEY_LEFT: {
            std::cout << "left pressed" << std::endl;
            std::cout << hitting_part_xz_plane_rotate_angle << std::endl;
            left_pressed = true;
            break;
        }

        case GLFW_KEY_UP: {
            std::cout << "up pressed" << std::endl;
            up_pressed = true;
            break;
        }

        case GLFW_KEY_DOWN: {
            std::cout << "down pressed" << std::endl;
            down_pressed = true;
            break;
        }

        case GLFW_KEY_SPACE: {
            std::cout << "space pressed" << std::endl;
            space_pressed = true;
            break;
        }

        case GLFW_KEY_I: {
            std::cout << "I (upward) pressed" << std::endl;
            std::cout << height_set << std::endl;
            upward_pressed = true;
            break;
        }

        case GLFW_KEY_O: {
            std::cout << "O (downward) pressed" << std::endl;
            std::cout << height_set << std::endl;
            downward_pressed = true;
            break;
        }

        case GLFW_KEY_ENTER: {
            // press enter for reset
            std::cout << "enter pressed" << std::endl;
            // ball
            ball_center_position = glm::vec3(2.0f, 0.25f, 2.0f);
            is_hit = false;
            is_rolling = false;
            ball_all_rotation.clear();
            last_ball_center_position = glm::vec3(2.0f, 0.25f, 2.0f);

            //putter
            height_set = 0.3f;
            putter_center_position = glm::vec3(0.0f, (0.0f + height_set), 0.0f);
            swing_angle = 0.0f;
            putter_swing_status = swing_status::NONE;

            // hitting part
            hitting_part_xz_plane_rotate_angle = 0.0f;
            hitting_part_center_position = glm::vec3(-0.5f, (0.4f + height_set), 0.0f);

            // rod
            rod_center_position = glm::vec3(0.0f, (3.0f + height_set), 0.0f);

            // key status
            up_pressed = false;
            down_pressed = false;
            right_pressed = false;
            left_pressed = false;
            space_pressed = false;
            space_release = false;
        }

        }
    }

    if (action == GLFW_RELEASE) {
        switch (key) {
        case GLFW_KEY_RIGHT: {
            std::cout << "right release" << std::endl;
            right_pressed = false;
            break;
        }

        case GLFW_KEY_LEFT: {
            std::cout << "left release" << std::endl;
            left_pressed = false;
            break;
        }

        case GLFW_KEY_UP: {
            std::cout << "up release" << std::endl;
            up_pressed = false;
            break;
        }

        case GLFW_KEY_DOWN: {
            std::cout << "down release" << std::endl;
            down_pressed = false;
            break;
        }

        case GLFW_KEY_I: {
            std::cout << "I (upward) release" << std::endl;
            std::cout << height_set << std::endl;
            upward_pressed = false;
            break;
        }

        case GLFW_KEY_O: {
            std::cout << "O (downward) release" << std::endl;
            std::cout << height_set << std::endl;
            downward_pressed = false;
            break;
        }

        case GLFW_KEY_SPACE: {
            std::cout << "space release" << std::endl;
            space_pressed = false;
            space_release = true;
            break;
        }

        }
    }
}

void initOpenGL() {
    // Initialize OpenGL context, details are wrapped in class.
#ifdef __APPLE__
  // MacOS need explicit request legacy support
    OpenGLContext::createContext(21, GLFW_OPENGL_ANY_PROFILE);
#else
  // OpenGLContext::createContext(21, GLFW_OPENGL_ANY_PROFILE);
    OpenGLContext::createContext(43, GLFW_OPENGL_COMPAT_PROFILE);
#endif
    GLFWwindow* window = OpenGLContext::getWindow();
    /* TODO#0: Change window title to "HW1 - `your student id`"
     *        Ex. HW1 - 311550000
     */
    glfwSetWindowTitle(window, "Enhanced Golf Game"); // name the window title CHANGE#0 --- change window title
    glfwSetKeyCallback(window, keyCallback);
    glfwSetFramebufferSizeCallback(window, resizeCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
#ifndef NDEBUG
    OpenGLContext::printSystemInfo();
    // This is useful if you want to debug your OpenGL API calls.
    OpenGLContext::enableDebugCallback();
#endif
}

void drawUnitSphere() {
    /* TODO#2-1: Render a unit sphere
     * Hint:
     *       glBegin/glEnd (https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/glBegin.xml)
     *       glColor3f (https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/glBegin.xml)
     *       glVertex3f (https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/glVertex.xml)
     *       glNormal (https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/glNormal.xml)
     *       glScalef (https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/glScale.xml)
     * Note:
     *       You can refer to ppt "Draw Sphere" page and `SECTOR` and `STACK`
     *       You can use color `CYAN` and `PURPLE`
     *       You should set normal for lighting
     *       You should use GL_TRIANGLES_STRIP
     */
    float sectorStep = 2 * M_PI / SECTOR;
    float stackStep = M_PI / STACK;

    // Loop through each stack
    for (int i = 0; i < STACK; ++i) {
        float stackAngle1 = M_PI / 2 - i * stackStep;       // Starting latitude angle
        float stackAngle2 = M_PI / 2 - (i + 1) * stackStep; // Next latitude angle

        float xy1 = cos(stackAngle1);  // Radius of the ring at stack 1
        float z1 = sin(stackAngle1);   // Z value at stack 1

        float xy2 = cos(stackAngle2);  // Radius of the ring at stack 2
        float z2 = sin(stackAngle2);   // Z value at stack 2

        // Set color based on stack level (upper or lower half)
        if (i < STACK / 2) {
            glColor3f(0.0f, 1.0f, 1.0f);  // CYAN for upper half
        }
        else {
            glColor3f(1.0f, 0.0f, 1.0f);  // PURPLE for lower half
        }

        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0; j <= SECTOR; ++j) {
            float sectorAngle = j * sectorStep;  // Longitude angle

            // Vertex 1 (for the current stack)
            float x1 = xy1 * cos(sectorAngle);
            float y1 = xy1 * sin(sectorAngle);
            glNormal3f(x1, y1, z1);  // Normal for lighting
            glVertex3f(x1, y1, z1);  // Vertex position

            // Vertex 2 (for the next stack)
            float x2 = xy2 * cos(sectorAngle);
            float y2 = xy2 * sin(sectorAngle);
            glNormal3f(x2, y2, z2);  // Normal for lighting
            glVertex3f(x2, y2, z2);  // Vertex position
        }
        glEnd();
    }
}

void drawUnitCylinder() {
    /* TODO#2-2: Render a unit cylinder
     * Hint:
     *       glBegin/glEnd (https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/glBegin.xml)
     *       glColor3f (https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/glBegin.xml)
     *       glVertex3f (https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/glVertex.xml)
     *       glNormal (https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/glNormal.xml)
     *       glScalef (https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/glScale.xml)
     * Note:
     *       You can refer to ppt "Draw Cylinder" page and `CIRCLE_SEGMENT`
     *       You can use color `SILVER`
     *       You should set normal for lighting
     *       You should use GL_TRIANGLES
     */
    float radius = 1.0f;
    float height = 1.0f;
    float halfHeight = height / 2.0f; // Half of the cylinder’s height, to center it at the origin.
    float angleStep = (2.0f * M_PI) / CIRCLE_SEGMENT; // Angle step for dividing the circular base.

    glColor3f(0.75f, 0.75f, 0.75f); // Set the cylinder color to silver (RGB: 0.75, 0.75, 0.75).

    // Draw the side surface of the cylinder using triangles
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < CIRCLE_SEGMENT; ++i) {
        float theta1 = i * angleStep;
        float theta2 = (i + 1) % CIRCLE_SEGMENT * angleStep;

        // Calculate (x, z) coordinates for points on the circumference at two consecutive angles
        float x1 = radius * cos(theta1);
        float z1 = radius * sin(theta1);
        float x2 = radius * cos(theta2);
        float z2 = radius * sin(theta2);

        // Set normal for the side surface at theta1 to achieve smooth lighting
        glNormal3f(cos(theta1), 0.0f, sin(theta1));

        // First triangle for the side face
        glVertex3f(x1, -halfHeight, z1);
        glVertex3f(x1, halfHeight, z1);
        glVertex3f(x2, halfHeight, z2);

        // Second triangle for the side face
        glVertex3f(x1, -halfHeight, z1);
        glVertex3f(x2, halfHeight, z2);
        glVertex3f(x2, -halfHeight, z2);
    }
    glEnd();

    // Draw the top cap
    glBegin(GL_TRIANGLES);
    glNormal3f(0.0f, 1.0f, 0.0f); // Normal facing up (Y-axis)
    for (int i = 0; i < CIRCLE_SEGMENT; ++i) {
        float theta1 = i * angleStep;
        float theta2 = (i + 1) % CIRCLE_SEGMENT * angleStep;

        float x1 = radius * cos(theta1);
        float z1 = radius * sin(theta1);
        float x2 = radius * cos(theta2);
        float z2 = radius * sin(theta2);

        glVertex3f(0.0f, halfHeight, 0.0f);
        glVertex3f(x2, halfHeight, z2);
        glVertex3f(x1, halfHeight, z1);
    }
    glEnd();

    // Draw the bottom cap
    glBegin(GL_TRIANGLES);
    glNormal3f(0.0f, -1.0f, 0.0f); // Normal facing down (Y-axis)
    for (int i = 0; i < CIRCLE_SEGMENT; ++i) {
        float theta1 = i * angleStep;
        float theta2 = (i + 1) % CIRCLE_SEGMENT * angleStep;

        float x1 = radius * cos(theta1);
        float z1 = radius * sin(theta1);
        float x2 = radius * cos(theta2);
        float z2 = radius * sin(theta2);

        glVertex3f(0.0f, -halfHeight, 0.0f);
        glVertex3f(x1, -halfHeight, z1);
        glVertex3f(x2, -halfHeight, z2);
    }
    glEnd();
}

void light() {
    GLfloat light_specular[] = { 0.6, 0.6, 0.6, 1 };
    GLfloat light_diffuse[] = { 0.6, 0.6, 0.6, 1 };
    GLfloat light_ambient[] = { 0.4, 0.4, 0.4, 1 };
    GLfloat light_position[] = { 50.0, 75.0, 80.0, 1.0 };
    // z buffer enable
    glEnable(GL_DEPTH_TEST);
    // enable lighting
    glEnable(GL_LIGHTING);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_NORMALIZE);
    // set light property
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
}

void drawGolfBall() {
    // Draw the golf ball
    glPushMatrix();
    glTranslatef(ball_center_position.x, ball_center_position.y, ball_center_position.z); // move the ball to its position
    int ball_all_rotation_size = ball_all_rotation.size();
    // compute the rolling of the ball
    // because the operation is from the stack to the stack bottom
    // we should stack the rolling direction and the angle from the back of ball_all_rotation_size
    for (int i = ball_all_rotation_size - 1; i >= 0; i--) {
        glRotatef(ball_all_rotation[i].first, ball_all_rotation[i].second.x, ball_all_rotation[i].second.y, ball_all_rotation[i].second.z);
    }
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f); // make the golf ball look like the same with TA's 
    glScalef(0.25f, 0.25f, 0.25f); // scale the ball by radius
    drawUnitSphere();
    glPopMatrix();

}

void drawPutter() {
    // Draw the hitting part
    glPushMatrix();
    if (putter_swing_status != swing_status::NONE) {

        // the y coordination + 5.6 is because when doing swing calculation, we move the top of the rod to (0, 0, 0) before rotation 
        glTranslatef(hitting_part_center_position.x, hitting_part_center_position.y + 5.6f, hitting_part_center_position.z); // move the hitting part,
        glRotatef(hitting_part_xz_plane_rotate_angle, 0.0f, 1.0f, 0.0f); // rotate along y-axis by yawing angle 

        glTranslatef(0.0f, 0.0f, 0.0f); // move to (0, 0, 0) to calculate yawing rotation

        // swing calculation
        glRotatef(swing_angle, -1.0f, 0.0f, 0.0f);
        glTranslated(0.0f, -5.6f, 0.0f);
    }
    else {
        // if the putter is not swinging
        glTranslatef(hitting_part_center_position.x, hitting_part_center_position.y, hitting_part_center_position.z); // move the hitting part
        glRotatef(hitting_part_xz_plane_rotate_angle, 0.0f, 1.0f, 0.0f); // rotate along y-axis by yawing angle 
    }
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f); // rotate the hitting part along x-axis by 90 degrees
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f); // rotate the hitting part along y-axis by 90 degrees
    glScalef(0.4f, 0.5f, 1.0f); // scale the cylinder to make it look like a hitting part
    drawUnitCylinder();
    glPopMatrix();

    // Draw the rod part (the same with drawing the hitting part)
    glPushMatrix();
    if (putter_swing_status != swing_status::NONE) {
        glTranslatef(rod_center_position.x, rod_center_position.y + 3.0f, rod_center_position.z);
        glRotatef(hitting_part_xz_plane_rotate_angle, 0.0f, 1.0f, 0.0f);
        glTranslatef(0.0f, 0.0f, 0.0f);
        glRotatef(swing_angle, -1.0f, 0.0f, 0.0f);
        glTranslatef(0.0f, -3.0f, 0.0f);
    }
    else {
        glTranslatef(rod_center_position.x, rod_center_position.y, rod_center_position.z);
        glRotatef(hitting_part_xz_plane_rotate_angle, 0.0f, 1.0f, 0.0f);
    }
    glScalef(0.15f, 6.0f, 0.15f);
    drawUnitCylinder();
    glPopMatrix();
}


void update_putter() {

    if (space_pressed) {
        putter_swing_status = swing_status::BACK;
        swing_angle -= SWING_SPEED_BACK;
        if (swing_angle < CONSTRAIN_ANGLE) {
            swing_angle = CONSTRAIN_ANGLE;
        }
    }

    if (space_release) {
        // if the distance between the ball center and the hitting part center <= TOLERATE and the putter is not swinging and the ball is not rolling
        // the ball is hit
        glm::vec3 hitting_part_ball_distance_vector = ball_center_position - hitting_part_center_position;
        if (glm::length(hitting_part_ball_distance_vector) <= TOLERATE && putter_swing_status == swing_status::UP && is_rolling == false && swing_angle >= 0.0f) {
            is_hit = true;
        }


        up_pressed = down_pressed = left_pressed = right_pressed = false; //揮動過程中不能移動桿子
        upward_pressed = downward_pressed = false; //same here

        if (putter_swing_status == swing_status::BACK) {
            // if the putter is not swinging
            // set the putter swing status swinging upward
            putter_swing_status = swing_status::UP;
            swing_add_constraint = swing_angle;
            swing_angle += SWING_SPEED;
        }
        else if (putter_swing_status == swing_status::UP) {
            if ((swing_angle >= -1 * CONSTRAIN_ANGLE) || (swing_angle >= -1 * swing_add_constraint)) {
                // if the putter reach the limit
                // set the putter swing status swinging downward
                putter_swing_status = swing_status::DOWN;
                swing_angle -= SWING_SPEED;
            }
            else {
                // increase the swing angle by SWING_SPEED
                // and check whether it is over the limit
                swing_angle += SWING_SPEED;
                swing_angle = std::min(swing_angle, std::min((float)-1 * CONSTRAIN_ANGLE, (float)-1 * swing_add_constraint));
            }
        }
        else if (putter_swing_status == swing_status::DOWN) {
            if (swing_angle <= 0.0f) {
                // if the putter reach the limit
                // stop swinging
                putter_swing_status = swing_status::NONE;
                space_release = false;
            }
            else {
                // decrease the swing angle by SWING_SPEED
                // and check whether it is over the limit
                swing_angle -= SWING_SPEED;
                swing_angle = std::max(swing_angle, 0.0f);
            }
        }
    }

    if (up_pressed) {
        /*
          moving direction = (rod position - putter position) cross (hitting part position - putter position)
          move the hitting part position, rod position, and the putter position simultaneously by MOVING_SPEED along the moving direction
        */
        glm::vec3 delta_move = glm::normalize(glm::cross(rod_center_position - putter_center_position, hitting_part_center_position - putter_center_position)) * MOVING_SPEED;
        hitting_part_center_position += delta_move;
        rod_center_position += delta_move;
        putter_center_position += delta_move;
    }

    if (down_pressed) {
        /*
          moving direction = (rod position - putter position) cross (hitting part position - putter position)
          move the hitting part position, rod position, and the putter position simultaneously by MOVING_SPEED along the negative moving direction
        */
        glm::vec3 delta_move = glm::normalize(glm::cross(rod_center_position - putter_center_position, hitting_part_center_position - putter_center_position)) * MOVING_SPEED;
        hitting_part_center_position -= delta_move;
        rod_center_position -= delta_move;
        putter_center_position -= delta_move;
    }

    if (right_pressed) {
        hitting_part_xz_plane_rotate_angle += ROTATE_SPEED;

        hitting_part_center_position -= putter_center_position; // move the putter back to (0, 0, 0) because the rotation is along y-axis
        // Create a rotation matrix around the Y-axis
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), ANGLE_TO_RADIAN(ROTATE_SPEED), glm::vec3(0.0f, 1.0f, 0.0f));
        hitting_part_center_position = glm::vec3(rotationMatrix * glm::vec4(hitting_part_center_position, 1.0f)); // rotate the position of hitting part
        hitting_part_center_position += putter_center_position;
    }

    if (left_pressed) {
        // same with right key is triggered but the negative angle
        hitting_part_xz_plane_rotate_angle -= ROTATE_SPEED;

        hitting_part_center_position -= putter_center_position;
        // Create a rotation matrix around the Y-axis
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), ANGLE_TO_RADIAN(-ROTATE_SPEED), glm::vec3(0.0f, 1.0f, 0.0f));
        hitting_part_center_position = glm::vec3(rotationMatrix * glm::vec4(hitting_part_center_position, 1.0f));
        hitting_part_center_position += putter_center_position;
    }

    if (upward_pressed) {
        float height_move_y = std::max(0.0f, std::min(HEIGHT_SPEED, HEIGHT_UP_LIM - height_set));

        glm::vec3 height_move = glm::vec3(0.0f, height_move_y, 0.0f);

        hitting_part_center_position += height_move;
        rod_center_position += height_move;
        putter_center_position += height_move;

        height_set += height_move_y;
    }

    if (downward_pressed) {
        float height_move_y = std::max(0.0f, std::min(HEIGHT_SPEED, height_set - HEIGHT_DOWN_LIM));

        glm::vec3 height_move = glm::vec3(0.0f, -height_move_y, 0.0f);

        hitting_part_center_position += height_move;
        rod_center_position += height_move;
        putter_center_position += height_move;

        height_set -= height_move_y;
    }

}


void update_ball() {
    if (is_hit) {
        // if the ball is just hit, set it rolling 
        is_hit = false;
        is_rolling = true;
    }
    if (is_rolling) {
        if (glm::length(last_ball_center_position - ball_center_position) >= DISTANCE_TOLERANCE) {
            // if the ball is just stop, the update the last position as position now
            last_ball_center_position = ball_center_position;
            is_rolling = false;
        }
        else {
            // the moving direction is (rod position - putter position) cross (hitting part position - putter position), the same with putter moving direction
            glm::vec3 ball_move_directrion = glm::cross((rod_center_position - putter_center_position), (hitting_part_center_position - putter_center_position));
            glm::vec3 up(0.0f, 1.0f, 0.0f);
            //the rolling direction is the y-axis cross the moving direction
            glm::vec3 ball_rotation_axis = glm::cross(up, ball_move_directrion);


            ball_center_position += glm::normalize(ball_move_directrion) * BALL_MOVING_SPEED; // calculate the position of the ball

            float ball_rolling_angle = BALL_MOVING_SPEED * 360.0f * 2.0f / M_PI; // calculate the rolling angle, not based on ROLLING_SPEED, but according on BALL_MOVING_SPEED

            ball_all_rotation.push_back(std::make_pair(ball_rolling_angle, ball_rotation_axis)); // record the rolling direction and rolling angle

        }
    }
}






int main() {
    initOpenGL();
    GLFWwindow* window = OpenGLContext::getWindow();

    // Init Camera helper
    Camera camera(glm::vec3(0, 5, 10));
    camera.initialize(OpenGLContext::getAspectRatio());
    // Store camera as glfw global variable for callbasks use
    glfwSetWindowUserPointer(window, &camera);

    // Main rendering loop
    while (!glfwWindowShouldClose(window)) {
        // Polling events.
        glfwPollEvents();
        // Update camera position and view
        camera.move(window);
        // GL_XXX_BIT can simply "OR" together to use.
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        /// TO DO Enable DepthTest
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        // Projection Matrix
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(camera.getProjectionMatrix());
        // ModelView Matrix
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(camera.getViewMatrix());

#ifndef DISABLE_LIGHT
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearDepth(1.0f);
        light();
#endif







        /* TODO#4-1: Update putter
         *       1. Update xzpos with forward_vector.
         *       2. Update y_rotate.
         *       3. Update x_rotate(Swing). Remember that the constrain of `CONSTRAIN_ANGLE`
         *       4. Calculate forward_vector for next rendering loop.
         *
         * Hint:
         *      glm::normalize (
         * Note:
         *       You can use `ROTATE_SPEED` and `MOVING_SPEED` and `SWING_SPEED`
         * as the speed constant. If the rotate/movement/swing speed is too slow or too fast, please change `ROTATE_SPEED`
         * or `MOVING_SPEED` or `SWING_SPEED` value. You should finish keyCallback first and use variables include
         * "delta_" to update.
         */
        update_putter();






        /* TODO#4-2: Hit detection and Update golfball
         *       1. Find the position of the hitting part and the ball to calculate distance
         *       2. Determine whether the ball is hit(distance < TOLERANCE && putter is swinging)
         *
         *       if the ball is hit:
         *       3. Update ballpos with ball_forward.
         *       4. Update ball_rotate.
         *       5. Calculate the new ball_forward with forward_vector.
         *       6. Calculate the new ball_rotate_normal
         *       7. Calculate the new startpos
         *
         *       Implement ball stop:
         *       8. Determine whether the ball has to stop(distance >= DISTANCE_TOLERANCE)
         *       9. Update currentRotation
         * Hint:
         *      glm::mat4
         *      glm::translate
         *      glm::rotate
         *      glm::scale
         *      glm::length
         *      glm::normalize (
         * Note:
         *       You can use `BALL_MOVING_SPEED` and `ROLLING_SPEED`
         * as the speed constant. If the rotate/movement speed is too slow or too fast, please change `ROTATE_SPEED`
         * or `BALL_MOVING_SPEED` value. You should finish keyCallback first and use variables include
         * "delta_" to update.
         */
        update_ball();

        // Render a white plane
        glPushMatrix();
        glScalef(7, 1, 7);
        glBegin(GL_TRIANGLE_STRIP);
        glColor3f(1.0f, 1.0f, 1.0f);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(-5.0f, 0.0f, -5.0f);
        glVertex3f(-5.0f, 0.0f, 5.0f);
        glVertex3f(5.0f, 0.0f, -5.0f);
        glVertex3f(5.0f, 0.0f, 5.0f);
        glEnd();
        glPopMatrix();

        /* TODO#3-1: Render the putter
         *       1. Do rotate and translate for the putter.
         *       2. Render the hitting part.
         *       3. Render the rod part.
         * Hint:
         *       glPushMatrix/glPopMatrix (https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/glPushMatrix.xml)
         *       glRotatef (https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/glRotate.xml)
         *       glTranslatef (https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/glTranslate.xml)
         *       glColor3f (https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/glColor.xml)
         *       glScalef (https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/glScale.xml)
         * Note:
         *       The size of every component can refer to `Components size definition` section
         *       You may implement drawUnitCylinder first
         *       You should try and think carefully about changing the order of rotate and translate
         *       You can just use initial value for rotate and translate the whole putter before finishing TODO#3 and TODO#4
         */
        drawPutter();

        /* TODO#3-2: Render the golf ball
         *       1. Do rotate and translate for the ball.
         * Hint:
         *       glPushMatrix/glPopMatrix (https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/glPushMatrix.xml)
         *       glRotatef (https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/glRotate.xml)
         *       glTranslatef (https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/glTranslate.xml)
         *       glColor3f (https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/glColor.xml)
         *       glScalef (https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/glScale.xml)
         *       glMultMatrixf(https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/glMultMatrix.xml)
         *       glm::value_ptr()
         * Note:
         *       The size of every component can refer to `Components size definition` section
         *       You may implement drawUnitSphere first
         *       You should try and think carefully about changing the order of rotate and translate
         *       You can just use initial value for rotate and translate the whole putter before finishing TODO#3 and TODO#4
         */
        drawGolfBall();

#ifdef __APPLE__
        // Some platform need explicit glFlush
        glFlush();
#endif
        glfwSwapBuffers(window);
    }
    return 0;
}
