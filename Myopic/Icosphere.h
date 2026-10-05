#pragma once
#include <vector>
#include <unordered_map>

class Icosphere {
public:
	Icosphere(size_t subdiv) {
		float t = (1.0f + sqrtf(5.0f)) / 2.0f;
		index = 0;

		addVertex(-1, t, 0);
		addVertex(1, t, 0);
		addVertex(-1, -t, 0);
		addVertex(1, -t, 0);

		addVertex(0, -1, t);
		addVertex(0, 1, t);
		addVertex(0, -1, -t);
		addVertex(0, 1, -t);

		addVertex(t, 0, -1);
		addVertex(t, 0, 1);
		addVertex(-t, 0, -1);
		addVertex(-t, 0, 1);

		

		indices = std::vector<uint32_t>{
			// 5 faces around point 0
			0, 11, 5,
			0, 5, 1,
			0, 1, 7,
			0, 7, 10,
			0, 10, 11,

			// 5 adjacent faces
			1, 5, 9,
			5, 11, 4,
			11, 10, 2,
			10, 7, 6,
			7, 1, 8,

			// 5 faces around point 3
			3, 9, 4,
			3, 4, 2,
			3, 2, 6,
			3, 6, 8,
			3, 8, 9,

			// 5 adjacent faces
			4, 9, 5,
			2, 4, 11,
			6, 2, 10,
			8, 6, 7,
			9, 8, 1,
		};

		// refine triangles
		for (int n = 0; n < subdiv; n++)
		{
			std::vector<uint32_t> indices2;
			for(size_t i = 0; i < (indices.size() / 3); i++)
			{
				// replace triangle by 4 triangles
				int a = getMiddlePoint(indices[(i * 3)], indices[(i * 3) + 1]);
				int b = getMiddlePoint(indices[(i * 3) + 1], indices[(i * 3) + 2]);
				int c = getMiddlePoint(indices[(i * 3) + 2], indices[(i * 3)]);

				indices2.push_back(indices[(i * 3)]);
				indices2.push_back(a);
				indices2.push_back(c);

				indices2.push_back(indices[(i * 3) + 1]);
				indices2.push_back(b);
				indices2.push_back(a);

				indices2.push_back(indices[(i * 3) + 2]);
				indices2.push_back(c);
				indices2.push_back(b);

				indices2.push_back(a);
				indices2.push_back(b);
				indices2.push_back(c);

			}
			indices = indices2;
		}

	}

	std::vector<float>& verts() {
		return vertices;
	}

	std::vector<uint32_t>& inds() {
		return indices;
	}

private:

	std::vector<float> vertices;
	std::vector<uint32_t> indices;
    std::unordered_map<uint64_t, uint32_t> middlePoints;
    int index = 0;

	int addVertex(float x, float y, float z) {
        double length = sqrt(x * x + y * y + z * z);
        vertices.push_back(x / length);
        vertices.push_back(y / length);
        vertices.push_back(z / length);
        return index++;
	}

    // return index of point in the middle of p1 and p2
    int getMiddlePoint(int p1, int p2)
    {
        // first check if we have it already
        bool firstIsSmaller = p1 < p2;
        uint64_t smallerIndex = firstIsSmaller ? p1 : p2;
        uint64_t greaterIndex = firstIsSmaller ? p2 : p1;
        uint64_t key = (smallerIndex << 32) + greaterIndex;

        if (middlePoints.contains(key)) {
            return middlePoints.at(key);
        }

        float x = (vertices[(p1 * 3)] + vertices[(p2 * 3)]) / 2.0f;
        float y = (vertices[(p1 * 3) + 1] + vertices[(p2 * 3) + 1]) / 2.0f;
        float z = (vertices[(p1 * 3) + 2] + vertices[(p2 * 3) + 2]) / 2.0f;


        // add vertex makes sure point is on unit sphere
        int i = addVertex(x,y,z);

        // store it, return index
        middlePoints.insert({ key, i });
        return i;
    }
};