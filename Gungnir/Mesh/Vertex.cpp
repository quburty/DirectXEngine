#include "Framework.h"
#include "Vertex.h"

void VertexModel::AddIndexAndWeight(uint index, float weight)
{
	//weights 조사후, 크기에 따라 삽입

	for (int i = 0; i < MODEL_MAX_WEIGHTS_NUM; i++)
	{
		if (weight > weights[i])
		{
			for (int j = MODEL_MAX_WEIGHTS_NUM - 2; j >= i; j--)
			{
				weights[j + 1] = weights[j];
				indices[j + 1] = indices[j];
			}

			weights[i] = weight;
			indices[i] = index;

			break;
		}
	}
}
