#pragma once
#include <memory.h>
#include <map>
#include <string>
#include <iostream>

//Author:		Orlando Cazalet-Hyams
//Description:	This class is a pure base class
//				that allows sharing of loaded resources
//				to save memeory. Usually used in tandem 
//				with the ServiceLocator for ease of
//				access.
template <typename tResource>
class ResourceManager
{
public:
	struct ResourceData
	{
		tResource* m_resource;
		int m_refCount;
	};
private:
	std::map<std::string, ResourceData> m_map;

	tResource* findInstantiated(std::string _fname)
	{
		tResource* resource = nullptr;

		std::map<std::string, ResourceData>::iterator itr = m_map.find(_fname);
		//look to see if the asset already exists
		if (itr != m_map.end())
		{
			resource = itr->second.m_resource;
			itr->second.m_refCount++;
		}
		return resource;
	}
protected:
	virtual tResource* loadFromFile(std::string _fname) = 0;

public:
	ResourceManager() = default;
	virtual ~ResourceManager() 
	{ 
		for (auto& r : m_map) delete r.second.m_resource; 
	}
	//clear up unused resources to reduce memeroy footprint. 
	void removeUnusedResources()
	{ 
		for (auto& r : m_map)
		{
			if (r.second.m_refCount <= 0)
			{
				delete r.second.m_resource;
				m_map.erase(r);
			}
		}
	}

	void release(tResource* _resource)
	{
		std::map<std::string, ResourceData>::iterator itr;
		for (itr = m_map.begin(); itr != m_map.end(); itr++)
		{
			if (itr->second.m_resource == _resource)
			{
				if (--(itr->second.m_refCount) <= 0)
				{
					//delete resource and remove pair
					delete itr->second.m_resource;
					m_map.erase(itr, m_map.end());
				}
				break;
			}
		}
	}

	tResource* request(std::string _fname)
	{
		tResource* resource = findInstantiated(_fname);
		//if we couldn't find it...
		if (!resource)
		{
			//...load it with loader method
			resource = loadFromFile(_fname);
			//add a reference to the asset to the map
			ResourceData data;
			data.m_resource = resource;
			data.m_refCount = 1;
			m_map.insert(std::pair<std::string, ResourceData>(_fname, data));
		}
		
		return resource;
	}
	
};