#include "WindManager.h"
#include "Shorthand.h"
#include <algorithm>

void WindManager::updateResources()
{
}

bool WindManager::load(ID3D11Device* _device, int _maxRects, int _maxSpheres)
{


	return true;
}

void WindManager::unload()
{

}

WindRect* WindManager::CreateWindRect()
{
	if (m_rects.size() < m_maxRects)
	{
		m_rects.push_back(WindRect());
		return &m_rects.back();
	}
	return nullptr;
}

WindSphere* WindManager::CreateWindSphere()
{
	if (m_spheres.size() < m_maxRects)
	{
		m_spheres.push_back(WindSphere());
		return &m_spheres.back();
	}
	return nullptr;
}

void WindManager::remove(WindRect* _rect)
{
	ERASE_REMOVE_IF(m_rects, [_rect](const WindRect& _val) { return &_val == _rect;});
}

void WindManager::remove(WindSphere* _sphere)
{
	ERASE_REMOVE_IF(m_spheres, [_sphere](const WindSphere& _val) { return &_val == _sphere; });
}

void WindManager::removeAll()
{
	m_spheres.clear();
	m_spheres.shrink_to_fit();

	m_rects.clear();
	m_rects.shrink_to_fit();
}
