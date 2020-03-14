#include <SDL2/SDL.h>

#define WIDTH 300
#define HEIGHT 300

#define REGULAR_POINT_SIZE 4
#define CENTROID_SIZE 6

SDL_Window* window;
SDL_Renderer* renderer;

struct Color {
    Color(const unsigned r, const unsigned g, const unsigned b) : r(r), g(g), b(b) {}
    const unsigned r, g, b;
};

class Point {
  protected:
    Point(const unsigned x, const unsigned y, const Color& color, const SDL_Rect rect) : mX(x), mY(y), mColor(color), mRect(rect) {}
    const unsigned mX, mY;
    Color mColor;
    const SDL_Rect mRect;

  private:
    void draw() const { SDL_RenderFillRect(renderer, &mRect); }
    friend double distance(const Point& p1, const Point& p2);
    friend void connect(const Point& p1, const Point& p2, const Color& color);
};

double distance(const Point& p1, const Point& p2) {
    double dX = double(p1.mX) - p2.mX;
    double dY = double(p1.mY) - p2.mY;
    return sqrt(dX * dX + dY * dY);
}

void connect(const Point& p1, const Point& p2, const Color& color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 0xFF);
    p1.draw();
    p2.draw();
    SDL_RenderDrawLine(renderer, p1.mX, p1.mY, p2.mX, p2.mY);
}

class RegularPoint : public Point {
  public:
    RegularPoint(const unsigned x, const unsigned y) :
        Point(
            x, y, {0, 0, 0}, 
            {
                x - (REGULAR_POINT_SIZE / 2), 
                y - (REGULAR_POINT_SIZE / 2), 
                REGULAR_POINT_SIZE, 
                REGULAR_POINT_SIZE 
            }
        ) {}
};

class Centroid : public Point {
  public:
    Centroid(const unsigned x, const unsigned y, const Color color) :
        Point(
            x, y, color,
            {
                x - (CENTROID_SIZE / 2),
                y - (CENTROID_SIZE / 2),
                CENTROID_SIZE,
                CENTROID_SIZE
            }
        ) {}
};

void init() {
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("K-Means", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, 0);
}

void quit() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void keep_window() {
    SDL_Event event;
    do {
        SDL_RenderPresent(renderer);
        SDL_PollEvent(&event);
    } while (!((event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) || event.type == SDL_QUIT));
}


