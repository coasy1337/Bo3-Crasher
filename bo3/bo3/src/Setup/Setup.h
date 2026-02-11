#pragma once
#include <Include.h>

class Setup {
public:
	Setup( );
	~Setup( ) = default;

private:
	void Initialize( );

	bool m_IsDebug = true;
};

inline std::unique_ptr<Setup> g_Setup;