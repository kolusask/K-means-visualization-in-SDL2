#include <algorithm>
#include <SDL2/SDL.h>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <set>
#include <vector>

#define K 18
#define N_POINTS 2000

#define WIDTH 600
#define HEIGHT 600

#define REGULAR_POINT_SIZE 4
#define CENTROID_SIZE 6

#define DELAY 40

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
    Color color() const { return mColor; }
    virtual int x() const = 0;
    virtual int y() const = 0;
    bool operator<(const Point& other) const { return this->mId < other.mId; }

  protected:
    Point(const Color& color, const SDL_Rect rect) : mColor(color), mRect(rect), mId(sCount++) {}
    Color mColor;
    const SDL_Rect mRect;
    const unsigned mId;

  private:
    static unsigned sCount;
    void draw() const { SDL_RenderFillRect(renderer, &mRect); }
    friend double distance(const Point& p1, const Point& p2);
    friend void connect(const Point& p1, const Point& p2, const Color& color);
};

unsigned Point::sCount = 0;

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
    bool set(const int x, const int y) const {
        const bool changed = x != mX || y != mY;
        mX = x;
        mY = y;
        return changed;
    }
    
  private:
    int x() const override { return mX; }
    int y() const override { return mY; }
    mutable int mX, mY;
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

void put_points(std::set<std::shared_ptr<Centroid>>& centroids, std::set<RegularPoint>& points) {
    srand(time(nullptr));
    for (int i = 0; i < K; i++) {
        const int x = rand() % WIDTH;
        const int y = rand() % HEIGHT;
        const Color color = ColorProvider::get();
        centroids.insert(std::make_shared<Centroid>(x, y, color));
    }
    for (int i = 0; i < N_POINTS; i++) {
        const int x = rand() % WIDTH;
        const int y = rand() % HEIGHT;
        points.insert(RegularPoint(x, y));
    }
}

std::map<std::shared_ptr<Centroid>, std::set<RegularPoint>> assign_centroids(const std::set<std::shared_ptr<Centroid>>& centroids, const std::set<RegularPoint>& points) {
    std::map<std::shared_ptr<Centroid>, std::set<RegularPoint>> centrMap;
    for (const auto& c : centroids)
        centrMap.insert({c, std::set<RegularPoint>()});
    for (auto p : points) {
        auto& best = *std::min_element(
            centroids.begin(), 
            centroids.end(), 
            [&p](const std::shared_ptr<Centroid>& c1, const std::shared_ptr<Centroid>& c2) -> bool {
                return distance(p, *c1) < distance(p, *c2);
            }
        );
        centrMap[best].insert(p);
    }
    return centrMap;
}

void draw(std::map<std::shared_ptr<Centroid>, std::set<RegularPoint>>& map) {
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(renderer);
    for (const auto& entry : map)
        for (const auto& p : entry.second)
            connect(p, *entry.first, entry.first->color());
    SDL_RenderPresent(renderer);
}

void calculate_mean_point(const std::set<RegularPoint>& points, int& x, int& y) {
    long sumX = std::accumulate(
        points.begin(), 
        points.end(), 
        0, 
        [](const int x, const Point& p) -> long {
            return x + p.x();
        }
    );
    x = sumX / points.size();
    long sumY = std::accumulate(
        points.begin(), 
        points.end(), 
        0, 
        [](const int y, const Point& p) -> long {
            return y + p.y();
        }
    );
    y = sumY / points.size();
}

bool set_centroids(std::map<std::shared_ptr<Centroid>, std::set<RegularPoint>>& map, const std::set<RegularPoint>& points) {
    int x, y;
    bool changed = false;
    for (const auto& entry : map) {
        calculate_mean_point(entry.second, x, y);
        changed = entry.first->set(x, y) || changed;
    }
    return changed;
}

int main() {
    init();
    std::set<std::shared_ptr<Centroid>> centroids;
    std::set<RegularPoint> points;
    put_points(centroids, points);
    auto centrMap = assign_centroids(centroids, points);
    draw(centrMap);
    bool stop = false;
    while (!stop) {
        centrMap = assign_centroids(centroids, points);
        draw(centrMap);
        SDL_Delay(DELAY);
        stop = !set_centroids(centrMap, points);
    }
    keep_window();
    quit();
    return 0;
}
