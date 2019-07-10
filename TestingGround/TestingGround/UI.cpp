#include "MansInterfacin.h"

#define daddy ((MansInterfacin*)((char*)this - offsetof(MansInterfacin, ui)))

MansInterfacin::UI::UI(MansInterfacin::GraphicsSystem* graphics_system)
	: graphics_system(graphics_system)
{}

MansInterfacin::UI::Texture2D_ID MansInterfacin::UI::CreateTexture2D(unsigned width, unsigned height, ResourceModifyFreq modfreq, Texture2D::TextureFormat format, uint8_t* init_buffer){
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

	if (modfreq == ResourceModifyFreq::NEVER) {																						//if the modification rate is never and the resource is immutable
		texture_description.Usage = D3D11_USAGE_IMMUTABLE;

		if (!init_buffer)return -1;																									//fail if no buffer was specified, it needs to be initialized on creation
		D3D11_SUBRESOURCE_DATA subr_data = { init_buffer, width * 4, 0 };
		if (FAILED(this->graphics_system->device->CreateTexture2D(&texture_description, &subr_data, &newone.d3d_obj)))return -1;
	} else if(modfreq == ResourceModifyFreq::SOMETIMES){
		texture_description.Usage = D3D11_USAGE_DEFAULT;

		D3D11_SUBRESOURCE_DATA subr_data = { init_buffer, width * 4, 0 };															//TODO: FIX THIS FOR OTHER FORMATS
		if (FAILED(this->graphics_system->device->CreateTexture2D(&texture_description, (init_buffer)?&subr_data:0,	&newone.d3d_obj)))return -1;
	} else {
		texture_description.Usage = D3D11_USAGE_DYNAMIC;
		texture_description.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		if (FAILED(this->graphics_system->device->CreateTexture2D(&texture_description,	nullptr, &newone.d3d_obj)))return -1;
		if (init_buffer) {
			D3D11_MAPPED_SUBRESOURCE mapped_subresource;
			if (FAILED(this->graphics_system->context->Map(newone.d3d_obj, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mapped_subresource)))return -1;
			memcpy(mapped_subresource.pData, init_buffer, newone.width * newone.height * 4);
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
	return this->textures.size()-1;
}

bool MansInterfacin::UI::UpdateTexture2D(Texture2D_ID textureID, uint8_t* buffer){
	if (textureID == -1)return false;
	Texture2D* texture = &this->textures[textureID];
	switch (texture->modfreq) {
		case ResourceModifyFreq::NEVER: {
			return false;
			break;
		}
		case ResourceModifyFreq::SOMETIMES: {
			this->graphics_system->context->UpdateSubresource(texture->d3d_obj, 0, nullptr, buffer, texture->width * 4, 0);
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
			memcpy(mapped_subresource.pData, buffer, texture->width * texture->height * 4);
			this->graphics_system->context->Unmap(texture->d3d_obj, NULL);
			break;
		}
	}
	return true;
}

void MansInterfacin::UI::SetTexture(Texture2D_ID textureID){
	if (textureID == -1)return;
	this->graphics_system->context->PSSetShaderResources(0, 1, &this->textures[textureID].d3d_obj_resourceview);

}

MansInterfacin::UI::VertexBuffer_ID MansInterfacin::UI::CreateVertexBuffer(unsigned element_size, unsigned buffer_size, ResourceModifyFreq modfreq, uint8_t* init_buffer){
	VertexBuffer newone = {};
	newone.buffer_size = buffer_size;
	newone.element_size = element_size;
	newone.modfreq = modfreq;


	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth = buffer_size * element_size;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	
	if (modfreq == ResourceModifyFreq::ALWAYS) {
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		if (FAILED(this->graphics_system->device->CreateBuffer(&bd, nullptr, &newone.d3d_obj)))return -1;

		if(init_buffer){
			D3D11_MAPPED_SUBRESOURCE mapped_subresource;
			if (FAILED(this->graphics_system->context->Map(newone.d3d_obj, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mapped_subresource)))return false;
			memcpy(mapped_subresource.pData, init_buffer, newone.buffer_size * newone.element_size);
			this->graphics_system->context->Unmap(newone.d3d_obj, NULL);
		}
	} else if (modfreq == ResourceModifyFreq::SOMETIMES) {
		bd.Usage = D3D11_USAGE_DEFAULT;

		D3D11_SUBRESOURCE_DATA sd = {init_buffer, 0, 0};
		if (FAILED(this->graphics_system->device->CreateBuffer(&bd, (init_buffer)?&sd:0, &newone.d3d_obj)))return -1;
	} else if (modfreq == ResourceModifyFreq::NEVER) {
		bd.Usage = D3D11_USAGE_IMMUTABLE;

		if (!init_buffer)return -1;

		D3D11_SUBRESOURCE_DATA sd = { init_buffer, 0, 0 };
		if (FAILED(this->graphics_system->device->CreateBuffer(&bd, &sd, &newone.d3d_obj)))return -1;
	}

	this->buffers.push_back(newone);
	return this->buffers.size()-1;
}

bool MansInterfacin::UI::UpdateVertexBuffer(VertexBuffer_ID bufferID, uint8_t* data){
	if (bufferID == -1)return false;
	VertexBuffer* buffer = &this->buffers[bufferID];
	switch (buffer->modfreq) {
		case ResourceModifyFreq::NEVER: {
			return false;
			break;
		}
		case ResourceModifyFreq::SOMETIMES: {
			this->graphics_system->context->UpdateSubresource(buffer->d3d_obj, 0, nullptr, data, 0, 0);
			break;
		}
		case ResourceModifyFreq::ALWAYS: {
			D3D11_MAPPED_SUBRESOURCE mapped_subresource;
			if (FAILED(this->graphics_system->context->Map(
				buffer->d3d_obj,
				NULL,
				D3D11_MAP_WRITE_DISCARD,
				NULL,
				&mapped_subresource
			)))return false;
			memcpy(mapped_subresource.pData, data, buffer->buffer_size*buffer->element_size);
			this->graphics_system->context->Unmap(buffer->d3d_obj, NULL);
			break;
		}
	}
	return true;
}

void MansInterfacin::UI::SetBuffers(VertexBuffer_ID* bufferIDs, uint32_t amount){
	std::vector<ID3D11Buffer*> buffers_d3d(amount);
	std::vector<uint32_t> strides_d3d(amount);
	std::vector<uint32_t> offsets_d3d(amount);
	for (uint32_t i = 0; i < amount; i++) {
		if (bufferIDs[i] == -1)return;
		buffers_d3d[i] = (this->buffers[bufferIDs[i]].d3d_obj);
		strides_d3d[i] = (this->buffers[bufferIDs[i]].element_size);
		offsets_d3d[i] = 0;
	}
	this->graphics_system->context->IASetVertexBuffers(0, amount, buffers_d3d.data(), strides_d3d.data(), offsets_d3d.data());
}

