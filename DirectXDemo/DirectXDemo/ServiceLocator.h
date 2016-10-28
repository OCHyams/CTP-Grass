#pragma once
#include <assert.h>

namespace OCH
{
	//Author:		Orlando Cazalet-Hyams
	/////////////////////////////////////////////////
	/// Simple templated service locator,
	///	it gives a global handle to an object.
	/////////////////////////////////////////////////
	template <typename tService>
	class ServiceLocator
	{
	public:
		/////////////////////////////////////////////////
		/// Add instance pointer
		/////////////////////////////////////////////////
		static void add(tService* _service)
		{
			//Don't want to overwirte the current instance
			assert(!instance().m_hasService);
			instance().m_service = _service;
			instance().m_hasService = true;
		}
		/////////////////////////////////////////////////
		/// Returns instance pointer
		/////////////////////////////////////////////////
		static tService* get()
		{
			//make sure you've instantiated the service!
			assert(instance().m_hasService);
			return instance().m_service;
		}

		/////////////////////////////////////////////////
		/// Remove the current instance and return its pointer
		/////////////////////////////////////////////////
		static tService* remove()
		{
			//can't remove it if it isn't there!
			assert(instance().m_hasService);
			//save temp copy of address
			tService* temp = instance().m_service;
			instance().m_service = nullptr;
			instance().m_hasService = false;

			return temp;
		}

		/////////////////////////////////////////////////
		/// Remove the current instance if it's the same as _service
		///	and return whether it happened
		/////////////////////////////////////////////////
		static bool remove(tService* _service)
		{
			if (instance().m_hasService && instance().m_service == _service)
			{
				instance().m_service = nullptr;
				instance().m_hasService = false;
				return true;
			}
			return false;
		}

	protected:
		bool m_hasService = false;
		tService* m_service = nullptr;

		ServiceLocator() = default;
		static ServiceLocator& instance()
		{
			static ServiceLocator<tService> inst;
			return inst;
		}
	};
}