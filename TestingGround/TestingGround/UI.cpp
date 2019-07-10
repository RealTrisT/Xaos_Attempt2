#include "MansInterfacin.h"

#define daddy ((MansInterfacin*)((char*)this - offsetof(MansInterfacin, ui)))

MansInterfacin::UI::UI(MansInterfacin::GraphicsSystem* graphics_system)
	: graphics_system(graphics_system)
{}

MansInterfacin::UI::Texture2D* MansInterfacin::UI::createTexture2D(unsigned width, unsigned height, ResourceModifyFreq modfreq, Texture2D::TextureFormat format, uint8_t* init_buffer){
	Texture2D newone = {};
	newone.width = width;		newone.height = height;
	newone.modfreq = modfreq;	newone.format = format;

	D3D11_TEXTURE2D_DESC texture_description = {};
	texture_description.Width = width;
	texture_description.Height = height;
	texture_description.MipLevels = 1;
	texture_description.ArraySize = 1;
	texture_description.SampleDesc.Count = 1;
	texture_description.SampleDesc.Quality = 0;
	texture_description.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texture_description.CPUAccessFlags = 0;
	texture_description.MiscFlags = 0;
	texture_description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;																		//TODO: FIX THIS FOR OTHER FORMATS
	switch (modfreq) {
	case ResourceModifyFreq::NEVER:
		texture_description.Usage = D3D11_USAGE_IMMUTABLE;
		break;
	case ResourceModifyFreq::SOMETIMES:
		texture_description.Usage = D3D11_USAGE_DEFAULT;
		break;
	case ResourceModifyFreq::ALWAYS:
		texture_description.Usage = D3D11_USAGE_DYNAMIC;
		break;
	}

	if (modfreq == ResourceModifyFreq::NEVER) {																						//if the modification rate is never and the resource is immutable
		if (!init_buffer)return 0;																									//fail if no buffer was specified, we
		D3D11_SUBRESOURCE_DATA subr_data = { init_buffer, width * 4, 0 };
		if (FAILED(this->graphics_system->device->CreateTexture2D(																	// init the texture. If it fails, fail.
			&texture_description, &subr_data, 
			&newone.the_texture
		)))return 0;
	} else {
		D3D11_SUBRESOURCE_DATA subr_data = { init_buffer, width * 4, 0 };															//TODO: FIX THIS FOR OTHER FORMATS
		if (FAILED(this->graphics_system->device->CreateTexture2D(
			&texture_description, 
			(init_buffer)?&subr_data:0,																								//same thing here, but only if there's anything in the buffer do we initialize
			&newone.the_texture
		)))return 0;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC texture_resourceview_description = {};
	texture_resourceview_description.Format = texture_description.Format;
	texture_resourceview_description.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	texture_resourceview_description.Texture2D.MostDetailedMip = 0;
	texture_resourceview_description.Texture2D.MipLevels = -1;

	if (FAILED(this->graphics_system->device->CreateShaderResourceView(
		newone.the_texture, 
		&texture_resourceview_description, 
		&newone.the_texture_resourceview
	)))return 0;

	this->textures.push_back(newone);
	return &this->textures.back();
}

void MansInterfacin::UI::SetTexture(Texture2D* texture){
	this->graphics_system->context->PSSetShaderResources(0, 1, &texture->the_texture_resourceview);

}
