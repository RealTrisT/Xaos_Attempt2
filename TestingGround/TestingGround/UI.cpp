#include "MansInterfacin.h"

#define daddy ((MansInterfacin*)((char*)this - offsetof(MansInterfacin, ui)))

MansInterfacin::UI::UI(MansInterfacin::GraphicsSystem* graphics_system)
	: graphics_system(graphics_system)
{}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------TEXTURE 2D-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//---------------------CREATE

DXGI_FORMAT format_dxformat[] = { 
	DXGI_FORMAT_R8G8B8A8_UNORM, 
	DXGI_FORMAT_A8_UNORM 
};
uint8_t format_size[]  = {
	4, 
	1
};

MansInterfacin::UI::Texture2D* MansInterfacin::UI::Create2DTexture(unsigned width, unsigned height, ResourceModifyFreq modfreq, Texture2D::TextureFormat format, uint8_t* init_buffer){
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
	texture_description.Format = format_dxformat[(uint8_t)format];

	uint8_t pixel_size = format_size[(uint8_t)format];

	if (modfreq == ResourceModifyFreq::NEVER) {																						//if the modification rate is never and the resource is immutable
		texture_description.Usage = D3D11_USAGE_IMMUTABLE;

		if (!init_buffer)return 0;																									//fail if no buffer was specified, it needs to be initialized on creation
		D3D11_SUBRESOURCE_DATA subr_data = { init_buffer, width * pixel_size, 0 };
		if (FAILED(this->graphics_system->device->CreateTexture2D(&texture_description, &subr_data, &newone.d3d_obj)))return 0;
	} else if(modfreq == ResourceModifyFreq::SOMETIMES){
		texture_description.Usage = D3D11_USAGE_DEFAULT;

		D3D11_SUBRESOURCE_DATA subr_data = { init_buffer, width * pixel_size, 0 };
		if (FAILED(this->graphics_system->device->CreateTexture2D(&texture_description, (init_buffer)?&subr_data:0,	&newone.d3d_obj)))return 0;
	} else {
		texture_description.Usage = D3D11_USAGE_DYNAMIC;
		texture_description.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		if (FAILED(this->graphics_system->device->CreateTexture2D(&texture_description,	nullptr, &newone.d3d_obj)))return 0;
		if (init_buffer) {
			D3D11_MAPPED_SUBRESOURCE mapped_subresource;
			if (FAILED(this->graphics_system->context->Map(newone.d3d_obj, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mapped_subresource)))return 0;
			memcpy(mapped_subresource.pData, init_buffer, newone.width * newone.height * pixel_size);
			this->graphics_system->context->Unmap(newone.d3d_obj, NULL);
		}
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC texture_resourceview_description = {};
	texture_resourceview_description.Format = texture_description.Format;
	texture_resourceview_description.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	texture_resourceview_description.Texture2D.MostDetailedMip = 0;
	texture_resourceview_description.Texture2D.MipLevels = -1;

	if (FAILED(this->graphics_system->device->CreateShaderResourceView(
		newone.d3d_obj, 
		&texture_resourceview_description, 
		&newone.d3d_obj_resourceview
	)))return 0;

	this->textures.push_back(newone);
	return &this->textures.back();
}


//---------------------UPDATE

bool MansInterfacin::UI::Update2DTexture(Texture2D* texture, uint8_t* buffer){
	if (!texture)return false;

	uint8_t pixel_size = format_size[(uint8_t)texture->format];

	switch (texture->modfreq) {
		case ResourceModifyFreq::NEVER: {
			return false;
			break;
		}
		case ResourceModifyFreq::SOMETIMES: {
			this->graphics_system->context->UpdateSubresource(texture->d3d_obj, 0, nullptr, buffer, texture->width * pixel_size, 0);
			break;
		}
		case ResourceModifyFreq::ALWAYS: {
			D3D11_MAPPED_SUBRESOURCE mapped_subresource;
			if (FAILED(this->graphics_system->context->Map(
				texture->d3d_obj, 
				NULL, 
				D3D11_MAP_WRITE_DISCARD, 
				NULL, 
				&mapped_subresource
			)))return false;
			memcpy(mapped_subresource.pData, buffer, texture->width * texture->height * pixel_size);
			this->graphics_system->context->Unmap(texture->d3d_obj, NULL);
			break;
		}
	}
	return true;
}

void MansInterfacin::UI::Set2DTexture(Texture2D* texture){
	if (!texture)return;
	this->graphics_system->context->PSSetShaderResources(0, 1, &texture->d3d_obj_resourceview);
}

void MansInterfacin::UI::Destroy2DTexture(Texture2D* texture) {  //TODO: Fix this for when the texture is in use. Worth mentioning as well that release has a return value. This should be bool.
	if (!texture)return;
	texture->d3d_obj_resourceview->Release();
	texture->d3d_obj->Release();

	//TODO: this is provisory, we need to remove it from the lined list
	texture->d3d_obj_resourceview = 0;
	texture->d3d_obj = 0;
}

MansInterfacin::UI::VertexBuffer* CreateD3DBuffer(
			MansInterfacin::UI* instance, 
			unsigned element_size, 
			unsigned buffer_size, 
			MansInterfacin::UI::ResourceModifyFreq modfreq, 
			uint8_t* init_buffer, 
			UINT bind_flags
)		{
#define this instance
	MansInterfacin::UI::VertexBuffer newone = {};
	newone.buffer_size = buffer_size;
	newone.element_size = element_size;
	newone.modfreq = modfreq;


	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth = buffer_size * element_size;
	bd.BindFlags = bind_flags;
	bd.CPUAccessFlags = 0;

	if (modfreq == MansInterfacin::UI::ResourceModifyFreq::ALWAYS) {
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		if (FAILED(this->graphics_system->device->CreateBuffer(&bd, nullptr, &newone.d3d_obj)))return 0;

		if (init_buffer) {
			D3D11_MAPPED_SUBRESOURCE mapped_subresource;
			if (FAILED(this->graphics_system->context->Map(newone.d3d_obj, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mapped_subresource)))return 0;
			memcpy(mapped_subresource.pData, init_buffer, newone.buffer_size * newone.element_size);
			this->graphics_system->context->Unmap(newone.d3d_obj, NULL);
		}
	}
	else if (modfreq == MansInterfacin::UI::ResourceModifyFreq::SOMETIMES) {
		bd.Usage = D3D11_USAGE_DEFAULT;

		D3D11_SUBRESOURCE_DATA sd = { init_buffer, 0, 0 };
		if (FAILED(this->graphics_system->device->CreateBuffer(&bd, (init_buffer) ? &sd : 0, &newone.d3d_obj)))return 0;
	}
	else if (modfreq == MansInterfacin::UI::ResourceModifyFreq::NEVER) {
		bd.Usage = D3D11_USAGE_IMMUTABLE;

		if (!init_buffer)return 0;

		D3D11_SUBRESOURCE_DATA sd = { init_buffer, 0, 0 };
		if (FAILED(this->graphics_system->device->CreateBuffer(&bd, &sd, &newone.d3d_obj)))return 0;
	}

	this->buffers.push_back(newone);
	return &this->buffers.back();
#undef this
}

bool UpdateD3DBuffer(MansInterfacin::UI* instance, MansInterfacin::UI::D3DBuffer* buffer, uint8_t* data) {
#define this instance
	if (!buffer)return false;

	switch (buffer->modfreq) {
		case MansInterfacin::UI::ResourceModifyFreq::NEVER: {
			return false;
			break;
		}
		case MansInterfacin::UI::ResourceModifyFreq::SOMETIMES: {
			this->graphics_system->context->UpdateSubresource(buffer->d3d_obj, 0, nullptr, data, 0, 0);
			break;
		}
		case MansInterfacin::UI::ResourceModifyFreq::ALWAYS: {
			D3D11_MAPPED_SUBRESOURCE mapped_subresource;
			if (FAILED(this->graphics_system->context->Map(
				buffer->d3d_obj,
				NULL,
				D3D11_MAP_WRITE_DISCARD,
				NULL,
				&mapped_subresource
			)))return false;
			memcpy(mapped_subresource.pData, data, buffer->buffer_size * buffer->element_size);
			this->graphics_system->context->Unmap(buffer->d3d_obj, NULL);
			break;
		}
	}
	return true;
#undef this
}

MansInterfacin::UI::VertexBuffer* MansInterfacin::UI::CreateVertexBuffer(unsigned element_size, unsigned buffer_size, ResourceModifyFreq modfreq, uint8_t* init_data){
	return CreateD3DBuffer(this, element_size, buffer_size, modfreq, init_data, D3D11_BIND_VERTEX_BUFFER);
}

bool MansInterfacin::UI::UpdateVertexBuffer(VertexBuffer* buffer, uint8_t* data){
	return UpdateD3DBuffer(this, buffer, data);
}

void MansInterfacin::UI::SetVertexBuffers(VertexBuffer** buffers, uint32_t amount){
	std::vector<ID3D11Buffer*> buffers_d3d(amount);
	std::vector<uint32_t> strides_d3d(amount);
	std::vector<uint32_t> offsets_d3d(amount);
	for (uint32_t i = 0; i < amount; i++) {
		if (!buffers[i])return;
		buffers_d3d[i] = (buffers[i]->d3d_obj);
		strides_d3d[i] = (buffers[i]->element_size);
		offsets_d3d[i] = 0;
	}
	this->graphics_system->context->IASetVertexBuffers(0, amount, buffers_d3d.data(), strides_d3d.data(), offsets_d3d.data());
}

void MansInterfacin::UI::DestroyVertexBuffer(VertexBuffer* buffer) {
	if (!buffer)return;
	buffer->d3d_obj->Release();

	//TODO: this is provisory, we need to remove it from the lined list
	buffer->d3d_obj = 0;
}

MansInterfacin::UI::IndexBuffer* MansInterfacin::UI::CreateIndexBuffer(unsigned element_size, unsigned buffer_size, ResourceModifyFreq modfreq, uint8_t* init_data){
	return CreateD3DBuffer(this, element_size, buffer_size, modfreq, init_data, D3D11_BIND_INDEX_BUFFER);
}

bool MansInterfacin::UI::UpdateIndexBuffer(IndexBuffer* buffer, uint8_t* data){
	return UpdateD3DBuffer(this, buffer, data);
}

const DXGI_FORMAT index_buffer_format_from_size[5] = {
	(DXGI_FORMAT)0,			//0
	DXGI_FORMAT_R8_UINT,	//1
	DXGI_FORMAT_R16_UINT,	//2
	(DXGI_FORMAT )0,		//3
	DXGI_FORMAT_R32_UINT,	//4
};

void MansInterfacin::UI::SetIndexBuffer(IndexBuffer* buffer){
	if (!buffer)this->graphics_system->context->IASetIndexBuffer(0, (DXGI_FORMAT)0, 0);
	else		this->graphics_system->context->IASetIndexBuffer(buffer->d3d_obj, index_buffer_format_from_size[buffer->element_size], 0);
}

void MansInterfacin::UI::DestroyIndexBuffer(IndexBuffer* buffer){
	if (!buffer)return;
	buffer->d3d_obj->Release();

	//TODO: this is provisory, we need to remove it from the linked list
	buffer->d3d_obj = 0;
}

