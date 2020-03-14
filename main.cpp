#include <SDL2/SDL.h>
#include <cstdlib>
#include <map>
#include <set>
#include <vector>

#define WIDTH 300
#define HEIGHT 300

#define K 3
#define N_POINTS 200

#define REGULAR_POINT_SIZE 4
#define CENTROID_SIZE 6

SDL_Window* window;
SDL_Renderer* renderer;

struct Color {
    Color(const int r, const int g, const int b) : r(r), g(g), b(b) {}
    const int r, g, b;
};

class ColorProvider {
  public:
    static Color get() {
        const int r = rand() % 256;
        const int g = rand() % 256;
        const int b = rand() % 256;
        return Color(r, g, b);
    }
};

class Point {
  public:
    bool operator<(const Point& other) const { return this->x() == other.x() ? this->y() < other.y() : this->x() < other.x(); }

  protected:
    Point(const Color& color, const SDL_Rect rect) : mColor(color), mRect(rect) {}
    virtual int x() const = 0;
    virtual int y() const = 0;
    Color mColor;
    const SDL_Rect mRect;

  private:
    void draw() const { SDL_RenderFillRect(renderer, &mRect); }
    friend double distance(const Point& p1, const Point& p2);
    friend void connect(const Point& p1, const Point& p2, const Color& color);
};

double distance(const Point& p1, const Point& p2) {
    double dX = double(p1.x()) - p2.x();
    double dY = double(p1.y()) - p2.y();
    return sqrt(dX * dX + dY * dY);
}

void connect(const Point& p1, const Point& p2, const Color& color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 0xFF);
    p1.draw();
    p2.draw();
    SDL_RenderDrawLine(renderer, p1.x(), p1.y(), p2.x(), p2.y());
}

class RegularPoint : public Point {
  public:
    RegularPoint(const int x, const int y) :
        Point(
            {0, 0, 0}, 
            {
                x - (REGULAR_POINT_SIZE / 2), 
                y - (REGULAR_POINT_SIZE / 2), 
                REGULAR_POINT_SIZE, 
                REGULAR_POINT_SIZE 
            }
        ),
        mX(x),
        mY(y) {}
  
  private:
    int x() const override { return mX; }
    int y() const override { return mY; }
    const int mX, mY;
};

class Centroid : public Point {
  public:
    Centroid(const int x, const int y, const Color color) :
        Point(
            color,
            {
                x - (CENTROID_SIZE / 2),
                y - (CENTROID_SIZE / 2),
                CENTROID_SIZE,
                CENTROID_SIZE
            }
        ),
        mX(x),
        mY(y) {}
    void set(const int x, const int y) {
        mX = x;
        mY = y;
    }
    
  private:
    int x() const override { return mX; }
    int y() const override { return mY; }
    int mX, mY;
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

void put_points(std::vector<Centroid>& centroids, std::set<RegularPoint>& points) {
    srand(time(nullptr));
    for (int i = 0; i < K; i++) {
        const int x = rand() % WIDTH;
        const int y = rand() % HEIGHT;
        const Color color = ColorProvider::get();
        centroids.push_back(Centroid(x, y, color));
    }
    for (int i = 0; i < N_POINTS; i++) {
        const int x = rand() % WIDTH;
        const int y = rand() % HEIGHT;
        points.insert(RegularPoint(x, y));
    }
}

std::map<Centroid, std::set<RegularPoint>> assign_centroids(const std::vector<Centroid>& centroids, const std::set<RegularPoint>& points) {
    std::map<Centroid, std::set<RegularPoint>> centrMap;
    for (const auto& c : centroids)
        centrMap.insert({c, std::set<RegularPoint>()});
    
}
