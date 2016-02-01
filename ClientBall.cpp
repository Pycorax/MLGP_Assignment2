#include "ClientBall.h"

#include "hge.h"
#include "hgesprite.h"

ClientBall::ClientBall(hgeSprite* sprite) : Ball()
	, server_w_(0)
	, client_w_(0)
	, server_velx_(0)
	, server_vely_(0)
	, ratio_(1)
	, sprite_(sprite)
{
}


ClientBall::~ClientBall()
{
}

void ClientBall::Update(float timedelta)
{
	HGE* hge = hgeCreate(HGE_VERSION);
	float pi = 3.141592654f * 2;

	server_w_ += angular_velocity * timedelta;

	if (server_w_ > pi)
		server_w_ -= pi;

	if (server_w_ < 0.0f)
		server_w_ += pi;

	client_w_ += angular_velocity * timedelta;

	if (client_w_ > pi)
		client_w_ -= pi;

	if (client_w_ < 0.0f)
		client_w_ += pi;

	w_ = ratio_ * server_w_ + (1 - ratio_) * client_w_;

	if (w_ > pi)
		w_ -= pi;

	if (w_ < 0.0f)
		w_ += pi;

	// store old coords
	oldx = x_;
	oldy = y_;

	x_ += velocity_x_ * timedelta;
	y_ += velocity_y_ * timedelta;

	float screenwidth = static_cast<float>(hge->System_GetState(HGE_SCREENWIDTH));
	float screenheight = static_cast<float>(hge->System_GetState(HGE_SCREENHEIGHT));
	float spritewidth = sprite_->GetWidth();
	float spriteheight = sprite_->GetHeight();

	server_x_ += server_velx_ * timedelta;
	server_y_ += server_vely_ * timedelta;

	if (server_x_ < -spritewidth / 2)
		server_x_ += screenwidth + spritewidth;
	else if (server_x_ > screenwidth + spritewidth / 2)
		server_x_ -= screenwidth + spritewidth;

	if (server_y_ < -spriteheight / 2)
		server_y_ += screenheight + spriteheight;
	else if (server_y_ > screenheight + spriteheight / 2)
		server_y_ -= screenheight + spriteheight;


	client_x_ += velocity_x_ * timedelta;
	client_y_ += velocity_y_ * timedelta;

	if (client_x_ < -spritewidth / 2)
		client_x_ += screenwidth + spritewidth;
	else if (client_x_ > screenwidth + spritewidth / 2)
		client_x_ -= screenwidth + spritewidth;

	if (client_y_ < -spriteheight / 2)
		client_y_ += screenheight + spriteheight;
	else if (client_y_ > screenheight + spriteheight / 2)
		client_y_ -= screenheight + spriteheight;

	if ((server_x_ < -spritewidth / 2 && client_x_ > screenwidth + spritewidth / 2) ||
		(server_x_ > screenwidth + spritewidth / 2 && client_x_ < -spritewidth / 2))
	{
		x_ = server_x_;
	}
	else
	{
		x_ = ratio_ * server_x_ + (1 - ratio_) * client_x_;
	}

	if ((server_y_ < -spriteheight / 2 && client_y_ > screenheight + spriteheight / 2) ||
		(server_y_ > screenheight + spriteheight / 2 && client_y_ < -spriteheight / 2))
	{
		y_ = server_y_;
	}
	else
	{
		y_ = ratio_ * server_y_ + (1 - ratio_) * client_y_;
	}

	if (ratio_ < 1)
	{
		// interpolating ratio step
		ratio_ += timedelta * 4;
		if (ratio_ > 1)
			ratio_ = 1;
	}

	if (x_ < -spritewidth / 2)
		x_ += screenwidth + spritewidth;
	else if (x_ > screenwidth + spritewidth / 2)
		x_ -= screenwidth + spritewidth;

	if (y_ < -spriteheight / 2)
		y_ += screenheight + spriteheight;
	else if (y_ > screenheight + spriteheight / 2)
		y_ -= screenheight + spriteheight;
}

void ClientBall::Render()
{
	sprite_->RenderEx(x_, y_, w_);
}

hgeRect* ClientBall::GetBoundingBox()
{
	sprite_->GetBoundingBox(x_, y_, &collidebox);

	return &collidebox;
}


bool ClientBall::HasCollided(Ship *ship)
{
	sprite_->GetBoundingBox(x_, y_, &collidebox);

	return collidebox.Intersect(ship->GetBoundingBox());
}

void ClientBall::SendObject(RakPeerInterface * peer, MyMsgIDs type)
{
	
}

void ClientBall::SendObject(RakNet::BitStream * bs, MyMsgIDs type) const
{
	switch (type)
	{
		case ID_COLLIDEBALL:
		{
			bs->Write(x_);
			bs->Write(y_);
			bs->Write(velocity_x_);
			bs->Write(velocity_y_);
			bs->Write(server_velx_);
			bs->Write(server_vely_);
		}
		break;
	}
}

void ClientBall::RecvObject(RakNet::BitStream* bs, MyMsgIDs type)
{
	switch (type)
	{
		case ID_COLLIDEBALL:
		{
			bs->Read(x_);
			bs->Read(y_);
			bs->Read(velocity_x_);
			bs->Read(velocity_y_);
			bs->Read(server_velx_);
			bs->Read(server_vely_);
		}
		break;

		case ID_UPDATEBALL:
		{
			bs->Read(server_x_);
			bs->Read(server_y_);
			bs->Read(server_w_);

			bs->Read(server_velx_);
			bs->Read(server_vely_);
			bs->Read(angular_velocity);

			// Interpolation
			DoInterpolateUpdate();
		}
		break;
	}
}