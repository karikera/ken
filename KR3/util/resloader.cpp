#include "stdafx.h"
#include "resloader.h"


kr::krb::File::File(const fchar_t* path) noexcept
{
	if (!krb_fopen(this, path, _TF("rb")))
	{
		param = nullptr;
		vftable = nullptr;
	}
}
kr::krb::File::~File() noexcept
{
	if (vftable != nullptr)
	{
		KrbFile::close();
	}
}

#ifdef _DEBUG
#pragma comment(lib, "ken-res-loaderd.lib")
#else
#pragma comment(lib, "ken-res-loader.lib")
#endif