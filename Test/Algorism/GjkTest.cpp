#include <functional>
#include <iostream>
#include <random>

#include "ConvexHull.hpp"
#include "Gjk.hpp"

using namespace My;
using namespace std;
using namespace std::placeholders;

using TestDataType = float;

int main(int argc, char** argv) {
    int point_num = 100;

    if (argc > 1) {
        point_num = atoi(argv[1]);
    }

    Polyhedron<TestDataType> A, B;
    {
        cout << "Generate Polyhedron A" << endl;
        default_random_engine generator;
        generator.seed(1);
        uniform_real_distribution<TestDataType> distribution(-1.0, 1.0);
        auto dice = std::bind(distribution, generator);

        ConvexHull<TestDataType> convex_hull;
        cout << "Points Generated:" << endl;
        for (auto i = 0; i < point_num; i++) {
            PointPtrf point_ptr =
                make_shared<Point<TestDataType>>(Point<TestDataType>{dice(), dice(), dice()});
            convex_hull.AddPoint(std::move(point_ptr));
        }

        while (convex_hull.Iterate())
            ;

        A = convex_hull.GetHull();
        cerr << "num of faces generated: " << A.Faces.size() << endl;
    }

    {
        cout << "Generate Polyhedron B" << endl;
        default_random_engine generator;
        generator.seed(200);
        uniform_real_distribution<TestDataType> distribution(0.6, 1.7);
        auto dice = std::bind(distribution, generator);

        ConvexHull<TestDataType> convex_hull;
        for (auto i = 0; i < point_num; i++) {
            PointPtr<TestDataType> point_ptr =
                make_shared<Point<TestDataType>>(Point<TestDataType>{dice(), dice(), dice()});
            convex_hull.AddPoint(std::move(point_ptr));
        }

        while (convex_hull.Iterate())
            ;

        B = convex_hull.GetHull();
        cerr << "num of faces generated: " << B.Faces.size() << endl;
    }

    SupportFunction<TestDataType> support_function_A = [=](auto&& arg1) {
        return ConvexPolyhedronSupportFunction(A, arg1);
    };
    SupportFunction<TestDataType> support_function_B = [=](auto&& arg1) {
        return ConvexPolyhedronSupportFunction(B, arg1);
    };
    PointListf simplex;
    Vector3f direction({1.0f, 0.0f, 0.0f});
    int intersected;
    while ((intersected = GjkIntersection(
                support_function_A, support_function_B, direction, simplex)) ==
           -1)
        cerr << "approximate direction: " << direction;

    switch (intersected) {
        case 1:
            cout << "A and B IS intersected" << endl;
            break;
        case 0:
            cout << "A and B is NOT intersected" << endl;
            break;
        default:
            assert(0);
    }

    return 0;
}