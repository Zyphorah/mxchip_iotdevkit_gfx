#include "GFXDisplay.h"

#include "font.h"
#include <Arduino.h>

const int GFXDisplay::FONT_WIDTH = ::FONT_WIDTH;
const int GFXDisplay::FONT_HEIGHT = ::FONT_HEIGHT;


void GFXDisplay::clear()
{
  memset(_buffer, 0, sizeof(_buffer));
}

void GFXDisplay::flush()
{
  draw(0, 0, 128, 8, _buffer);
}

void GFXDisplay::setPixel(int x, int y)
{
  if (static_cast<unsigned>(x) >= WIDTH || static_cast<unsigned>(y) >= HEIGHT)
    return;
  
  // Calcul d'index optimisé (sans branchless compliqué)
  int _byte = x + ((y >> 3) << 7);
  _buffer[_byte] |= (1 << (y & 7));
}

void GFXDisplay::drawText(int x, int y, const char *s)
{
  while (*s != 0) {
    char *c = font[(*s) - 0x20];
    for (int _y = 0; _y < FONT_HEIGHT; _y++) {
      byte fontRow = c[_y];
      for (int _x = 0; _x < FONT_WIDTH; _x++) {
        if (fontRow & (1 << _x)) {
          setPixel(x + _x, y + _y);
        }
      }
    }
    x += FONT_WIDTH;
    s++;
  }
}

void GFXDisplay::drawLine(int x0, int y0, int x1, int y1)
{
  int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = dx + dy, e2;

  while (true) {
    setPixel(x0, y0);
    if (x0 == x1 && y0 == y1) break;
    e2 = err << 1;  // e2 = 2 * err (shift plus rapide)
    if (e2 >= dy) { err += dy; x0 += sx; }
    if (e2 <= dx) { err += dx; y0 += sy; }
  }
}

void GFXDisplay::drawRect(int x0, int y0, int x1, int y1)
{
  drawLine(x0, y0, x0, y1);
  drawLine(x0, y1, x1, y1);
  drawLine(x1, y1, x1, y0);
  drawLine(x1, y0, x0, y0);
}

void GFXDisplay::drawCircle(int xm, int ym, int r)
{
  int x = -r, y = 0, err = 2 - 2 * r;

  while (x <= 0) {
    // Dessiner 8 octants en une seule itération
    setPixel(xm - x, ym + y);
    setPixel(xm + x, ym + y);
    setPixel(xm - y, ym - x);
    setPixel(xm + y, ym - x);
    setPixel(xm - y, ym + x);
    setPixel(xm + y, ym + x);
    setPixel(xm - x, ym - y);
    setPixel(xm + x, ym - y);

    int e2 = err;
    if (e2 <= y) {
      err += (++y << 1) + 1;
    }
    if (e2 > x || err > y) {
      err += (++x << 1) + 1;
    }
  }
}

void GFXDisplay::drawBitmap(int x, int y, int w, int h, const byte *bmp)
{
  // Éviter les modulos en utilisant des index directs
  const byte *p = bmp;
  for (int row = 0; row < h; row++) {
    for (int col = 0; col < w; col++) {
      if (*p++) {
        setPixel(x + col, y + row);
      }
    }
  }
}

void GFXDisplay::drawCube3D(int centerX, int centerY, int size, float angleX, float angleY, float angleAnim) {
    float half = size / 2.0f;
    float vertices[8][3] = {
        {-half, -half, -half}, {half, -half, -half}, {half, half, -half}, {-half, half, -half},
        {-half, -half, half},  {half, -half, half},  {half, half, half},  {-half, half, half}
    };

    // Rotation animée autour Z
    float cosA = cos(angleAnim), sinA = sin(angleAnim);

    // Rotation X/Y
    float cosX = cos(angleX), sinX = sin(angleX);
    float cosY = cos(angleY), sinY = sin(angleY);

    int projected[8][2];
    for (int i = 0; i < 8; i++) {
        float x = vertices[i][0], y = vertices[i][1], z = vertices[i][2];
        // Rotation Z (animation)
        float xz = x * cosA - y * sinA;
        float yz = x * sinA + y * cosA;
        // Rotation X
        float y1 = yz * cosX - z * sinX;
        float z1 = yz * sinX + z * cosX;
        // Rotation Y
        float x2 = xz * cosY + z1 * sinY;
        float z2 = -xz * sinY + z1 * cosY;
        // Projection 2D
        float scale = 80.0f / (z2 + 120.0f);
        projected[i][0] = centerX + int(x2 * scale);
        projected[i][1] = centerY + int(y1 * scale);
    }

    int edges[12][2] = {
        {0,1},{1,2},{2,3},{3,0},
        {4,5},{5,6},{6,7},{7,4},
        {0,4},{1,5},{2,6},{3,7}
    };

    for (int i = 0; i < 12; i++) {
        drawLine(projected[edges[i][0]][0], projected[edges[i][0]][1],
                         projected[edges[i][1]][0], projected[edges[i][1]][1]);
    }
}

void GFXDisplay::drawCenterCross(int size) {
    int centerX = WIDTH / 2;
    int centerY = HEIGHT / 2;
    
    drawLine(centerX - size, centerY, centerX + size, centerY);
    drawLine(centerX, centerY - size, centerX, centerY + size);
}