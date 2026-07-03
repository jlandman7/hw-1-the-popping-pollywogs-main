#include "rasterizer.h"
#include <cmath>

using namespace std;

namespace CGL {

  RasterizerImp::RasterizerImp(PixelSampleMethod psm, LevelSampleMethod lsm,
    size_t width, size_t height,
    unsigned int sample_rate) {
    this->psm = psm;
    this->lsm = lsm;
    this->width = width;
    this->height = height;
    this->sample_rate = sample_rate;

    sample_buffer.resize(width * height * sample_rate, Color::White);
  }

  // Used by rasterize_point and rasterize_line
  void RasterizerImp::fill_pixel(size_t x, size_t y, Color c) {
    // TODO: Task 2: You might need to this function to fix points and lines (such as the black rectangle border in test4.svg)
    // NOTE: You are not required to implement proper supersampling for points and lines
    // It is sufficient to use the same color for all supersamples of a pixel for points and lines (not triangles)


    sample_buffer[y * width + x] = c;
  }

  // Rasterize a point: simple example to help you start familiarizing
  // yourself with the starter code.
  void RasterizerImp::rasterize_point(float x, float y, Color color) {
    // fill in the nearest pixel
    int sx = (int)floor(x);
    int sy = (int)floor(y);

    // check bounds
    if (sx < 0 || sx >= width) return;
    if (sy < 0 || sy >= height) return;

    for (int n = 0; n < sample_rate; n++)
      sample_buffer[(sy * width + sx) * sample_rate + n] = color;
    return;
  }

  // Rasterize a line.
  void RasterizerImp::rasterize_line(float x0, float y0,
    float x1, float y1,
    Color color) {
    if (x0 > x1) {
      swap(x0, x1); swap(y0, y1);
    }

    float pt[] = { x0,y0 };
    float m = (y1 - y0) / (x1 - x0);
    float dpt[] = { 1,m };
    int steep = abs(m) > 1;
    if (steep) {
      dpt[0] = x1 == x0 ? 0 : 1 / abs(m);
      dpt[1] = x1 == x0 ? (y1 - y0) / abs(y1 - y0) : m / abs(m);
    }

    while (floor(pt[0]) <= floor(x1) && abs(pt[1] - y0) <= abs(y1 - y0)) {
      rasterize_point(pt[0], pt[1], color);
      pt[0] += dpt[0]; pt[1] += dpt[1];
    }
  }

  // Rasterize a triangle.
  void RasterizerImp::rasterize_triangle(float x0, float y0,
                                       float x1, float y1,
                                       float x2, float y2,
                                       Color color) {

    // Determine winding (positive = CCW)
    
    float winding =
        (x1 - x0) * (y2 - y0) -
        (y1 - y0) * (x2 - x0);

    // Bounding box
    int xmin = (int)floor(std::min({x0, x1, x2}));
    int xmax = (int)ceil (std::max({x0, x1, x2}));
    int ymin = (int)floor(std::min({y0, y1, y2}));
    int ymax = (int)ceil (std::max({y0, y1, y2}));

    const int N = std::sqrt(sample_rate);
    for (int y = ymin; y <= ymax; y++) {
      for (int x = xmin; x <= xmax; x++) {
        for (int n = 0; n < N; n++){
          float py = y + (n + 0.5f)/N;
          
          for (int m = 0; m < N; m++){
            float px = x + (m + 0.5f)/N;

            // Edge functions
            float e0 =
                (px - x0) * (y1 - y0) -
                (py - y0) * (x1 - x0);

            float e1 =
                (px - x1) * (y2 - y1) -
                (py - y1) * (x2 - x1);

            float e2 =
                (px - x2) * (y0 - y2) -
                (py - y2) * (x0 - x2);

            if (winding < 0) {
              if (e0 >= 0 && e1 >= 0 && e2 >= 0){
                  int sample_index = n*N + m;
                  sample_buffer[(y * width + x) * sample_rate + sample_index] = color;
              }
            }
            else {
              if (e0 <= 0 && e1 <= 0 && e2 <= 0) {
                  int sample_index = n*N + m;
                  sample_buffer[(y * width + x) * sample_rate + sample_index] = color;
              }
            }
          }
        }
      }
    }

    // TODO: Task 2: Update to implement super-sampled rasterization
  }


  void RasterizerImp::rasterize_interpolated_color_triangle(float x0, float y0, Color c0,
    float x1, float y1, Color c1,
    float x2, float y2, Color c2)
  {
    // TODO: Task 4: Rasterize the triangle, calculating barycentric coordinates and using them to interpolate vertex colors across the triangle
    // Hint: You can reuse code from rasterize_triangle
    
    // Determine winding (positive = CCW)

    float winding =
        (x1 - x0) * (y2 - y0) -
        (y1 - y0) * (x2 - x0);

    // Bounding box
    int xmin = (int)floor(std::min({x0, x1, x2}));
    int xmax = (int)ceil (std::max({x0, x1, x2}));
    int ymin = (int)floor(std::min({y0, y1, y2}));
    int ymax = (int)ceil (std::max({y0, y1, y2}));

    const int N = std::sqrt(sample_rate);
    for (int y = ymin; y <= ymax; y++) {
      for (int x = xmin; x <= xmax; x++) {
        for (int n = 0; n < N; n++){
          float py = y + (n + 0.5f)/N;
          
          for (int m = 0; m < N; m++){
            float px = x + (m + 0.5f)/N;

            Color color = c0 * ((x1 - px) * (y2 - py) - (y1 - py) * (x2 - px)) * (1.0f/winding) +
                    c1 * ((x2 - px) * (y0 - py) - (y2 - py) * (x0 - px)) * (1.0f/winding) +
                    c2 * ((x0 - px) * (y1 - py) - (y0 - py) * (x1 - px)) * (1.0f/winding);

            // Edge functions
            float e0 =
                (px - x0) * (y1 - y0) -
                (py - y0) * (x1 - x0);

            float e1 =
                (px - x1) * (y2 - y1) -
                (py - y1) * (x2 - x1);

            float e2 =
                (px - x2) * (y0 - y2) -
                (py - y2) * (x0 - x2);

            if (winding < 0) {
              if (e0 >= 0 && e1 >= 0 && e2 >= 0){
                  int sample_index = n*N + m;
                  sample_buffer[(y * width + x) * sample_rate + sample_index] = color;
              }
            }
            else {
              if (e0 <= 0 && e1 <= 0 && e2 <= 0) {
                  int sample_index = n*N + m;
                  sample_buffer[(y * width + x) * sample_rate + sample_index] = color;
              }
            }
          }
        }
      }
    }

  }


  void RasterizerImp::rasterize_textured_triangle(float x0, float y0, float u0, float v0,
    float x1, float y1, float u1, float v1,
    float x2, float y2, float u2, float v2,
    Texture& tex)
  {
    // TODO: Task 5: Fill in the SampleParams struct and pass it to the tex.sample function.
    // TODO: Task 6: Set the correct barycentric differentials in the SampleParams struct.
    // Hint: You can reuse code from rasterize_triangle/rasterize_interpolated_color_triangle
    /*
    struct SampleParams {
      Vector2D p_uv;
      Vector2D p_dx_uv, p_dy_uv;
      PixelSampleMethod psm;
      LevelSampleMethod lsm;
    };
    */
    float winding =
        (x1 - x0) * (y2 - y0) -
        (y1 - y0) * (x2 - x0);

    // Bounding box
    int xmin = (int)floor(std::min({x0, x1, x2}));
    int xmax = (int)ceil (std::max({x0, x1, x2}));
    int ymin = (int)floor(std::min({y0, y1, y2}));
    int ymax = (int)ceil (std::max({y0, y1, y2}));

    const int N = std::sqrt(sample_rate);
    for (int y = ymin; y <= ymax; y++) {
      for (int x = xmin; x <= xmax; x++) {
        for (int n = 0; n < N; n++){
          float py = y + (n + 0.5f)/N;
          
          for (int m = 0; m < N; m++){
            float px = x + (m + 0.5f)/N;
            Vector2D pxpy = Vector2D(px, py);

            // Edge functions
            float e0 =
                (px - x0) * (y1 - y0) -
                (py - y0) * (x1 - x0);

            float e1 =
                (px - x1) * (y2 - y1) -
                (py - y1) * (x2 - x1);

            float e2 =
                (px - x2) * (y0 - y2) -
                (py - y2) * (x0 - x2);

            if (winding < 0) {
              if (e0 >= 0 && e1 >= 0 && e2 >= 0){
                  int sample_index = n*N + m;
                  Vector2D uv = Vector2D(u0, v0) * ((x1 - px) * (y2 - py) - (y1 - py) * (x2 - px)) * (1.0f/winding) +
                                Vector2D(u1, v1) * ((x2 - px) * (y0 - py) - (y2 - py) * (x0 - px)) * (1.0f/winding) +
                                Vector2D(u2, v2) * ((x0 - px) * (y1 -py) - (y0 - py) * (x1 - px)) * (1.0f/winding);
                  Vector2D uv_dx = Vector2D(u0, v0) * ((x1 - px - 1) * (y2 - py) - (y1 - py) * (x2 - px - 1)) * (1.0f/winding) +
                                Vector2D(u1, v1) * ((x2 - px - 1) * (y0 - py) - (y2 - py) * (x0 - px - 1)) * (1.0f/winding) +
                                Vector2D(u2, v2) * ((x0 - px - 1) * (y1 - py) - (y0 - py) * (x1 - px - 1)) * (1.0f/winding);
                  Vector2D uv_dy = Vector2D(u0, v0) * ((x1 - px) * (y2 - py - 1) - (y1 - py - 1) * (x2 - px)) * (1.0f/winding) +
                                Vector2D(u1, v1) * ((x2 - px) * (y0 - py - 1) - (y2 - py - 1) * (x0 - px)) * (1.0f/winding) +
                                Vector2D(u2, v2) * ((x0 - px) * (y1 - py - 1) - (y0 - py - 1) * (x1 - px)) * (1.0f/winding);
                  
                  SampleParams sp;
                  sp.p_uv = uv;
                  sp.p_dx_uv = uv_dx;
                  sp.p_dy_uv = uv_dy;
                  sp.psm = psm;
                  sp.lsm = lsm;
                  Color c = tex.sample(sp);
                  sample_buffer[(y * width + x) * sample_rate + sample_index] = c;
              }
            }
            else {
              if (e0 <= 0 && e1 <= 0 && e2 <= 0) {
                  int sample_index = n*N + m;
                  Vector2D uv = Vector2D(u0, v0) * ((x1 - px) * (y2 - py) - (y1 - py) * (x2 - px)) * (1.0f/winding) +
                                Vector2D(u1, v1) * ((x2 - px) * (y0 - py) - (y2 - py) * (x0 - px)) * (1.0f/winding) +
                                Vector2D(u2, v2) * ((x0 - px) * (y1 -py) - (y0 - py) * (x1 - px)) * (1.0f/winding);
                  Vector2D uv_dx = Vector2D(u0, v0) * ((x1 - px - 1) * (y2 - py) - (y1 - py) * (x2 - px - 1)) * (1.0f/winding) +
                                Vector2D(u1, v1) * ((x2 - px - 1) * (y0 - py) - (y2 - py) * (x0 - px - 1)) * (1.0f/winding) +
                                Vector2D(u2, v2) * ((x0 - px - 1) * (y1 - py) - (y0 - py) * (x1 - px - 1)) * (1.0f/winding);
                  Vector2D uv_dy = Vector2D(u0, v0) * ((x1 - px) * (y2 - py - 1) - (y1 - py - 1) * (x2 - px)) * (1.0f/winding) +
                                Vector2D(u1, v1) * ((x2 - px) * (y0 - py - 1) - (y2 - py - 1) * (x0 - px)) * (1.0f/winding) +
                                Vector2D(u2, v2) * ((x0 - px) * (y1 - py - 1) - (y0 - py - 1) * (x1 - px)) * (1.0f/winding);
                  SampleParams sp;
                  sp.p_uv = uv;
                  sp.p_dx_uv = uv_dx;
                  sp.p_dy_uv = uv_dy;
                  sp.psm = psm;
                  sp.lsm = lsm;
                  Color c = tex.sample(sp);
                  sample_buffer[(y * width + x) * sample_rate + sample_index] = c;
              }
            }
          }
        }
      }
    }
  }

  void RasterizerImp::set_sample_rate(unsigned int rate) {
    // TODO: Task 2: You may want to update this function for supersampling support

    this->sample_rate = rate;
    this->sample_buffer.resize(width * height * rate, Color::White);
  }


  void RasterizerImp::set_framebuffer_target(unsigned char* rgb_framebuffer,
    size_t width, size_t height)
  {
    // TODO: Task 2: You may want to update this function for supersampling support

    this->width = width;
    this->height = height;
    this->rgb_framebuffer_target = rgb_framebuffer;
    this->sample_buffer.resize(width * height * sample_rate, Color::White);
  }


  void RasterizerImp::clear_buffers() {
    std::fill(rgb_framebuffer_target, rgb_framebuffer_target + 3 * width * height, 255);
    std::fill(sample_buffer.begin(), sample_buffer.end(), Color::White);
  }


  // This function is called at the end of rasterizing all elements of the
  // SVG file.  If you use a supersample buffer to rasterize SVG elements
  // for antialising, you could use this call to fill the target framebuffer
  // pixels from the supersample buffer data.
  //
  void RasterizerImp::resolve_to_framebuffer() {
    // TODO: Task 2: You will likely want to update this function for supersampling support


    for (int x = 0; x < width; ++x) {
      for (int y = 0; y < height; ++y) {
        Color col = Color::Black;
        
        for (int s = 0; s < sample_rate; ++s)
          col += (sample_buffer[(y*width + x)*sample_rate + s]);
        col = col * (1.0f / sample_rate);

        for (int k = 0; k < 3; ++k) {
          this->rgb_framebuffer_target[3 * (y * width + x) + k] = (&col.r)[k] * 255;
        }
      }
    }

  }

  Rasterizer::~Rasterizer() { }


}// CGL