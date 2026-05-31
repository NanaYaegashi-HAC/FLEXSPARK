#include"SizeBiggerC.h"
#include"Util/UtilNumber.h"
#include"Actor/System/GimmickDetect.h"

static const float SBC_Size{ 10.0f };
static const float StanTimeMax{ 90.0f };	//1.5秒動けない
static const GSvector2 SBCTextureSize{ 128.0f,128.0f };
static const float SBCScale{ 0.16f };
static const GSvector2 core_aria_left_top_{ -25.0f,37.5f };//コアの当たり判定範囲　左上
static const GSvector2 core_aria_right_bottom_{ 25.0f,-37.5f };//コアの当たり判定範囲　右上
GSvector2 change_size_{ 0.0f,0.0f };

SizeBiggerC::SizeBiggerC(IWorld* world, const GSvector3& position, const int delete_num) {

	world_ = world;
	name_ = "SizeBiggerC";
	tag_ = gimmick_tag_;
	collider_ = BoundingSphere{ SBC_Size * everything_scale_,GSvector3{0.0f,0.0f,0.0f} };
	actor_handle_ = world_->add_actor_handle();
	sbc_state_ = SBC_Wait;
	transform_.position(position);
	origin_position_ = transform_.position();
	image_size_ = SBCTextureSize;
	scale_ = everything_scale_ * SBCScale;
	delete_positon_id_ = delete_num;

}

void SizeBiggerC::update(float delta_time) {

	if ((world_->next_scene() == 4 && world_->stage_make_type() == 1) || world_->gimmick_delete_frag()) {

		die();

	}

	Actor* core = world_->find_actor(core_name_ + owner_name_);

	switch (sbc_state_) {

	case SBC_Wait:	//待機
		bigger_time = 0.0f;
		big_time = 0.0f;
		small_time = 0.0f;
		is_big_ = false;

		break;

	case SBC_Bigger://大きくなっている状態
		bigger_time += delta_time;
		if (bigger_time > 120.0f)
		{
			sbc_state_ = SBC_smaller;
		}

		break;

	case SBC_Big:
		is_big_ = true;

		change_size_.y += 0.1f;
		core->change_add_scale(change_size_);

		big_time += delta_time;

		if (big_time > 60.0f)
		{
			sbc_state_ = SBC_Bigger;
		}

		break;

	case SBC_smaller:
		change_size_.y -= 0.1f;
		core->change_add_scale(change_size_);

		small_time += delta_time;
		if (small_time > 60.0f)
		{
			sbc_state_ = SBC_Wait;
			if(delete_positon_id_!=-1)world_->add_actor(new GimmickDetect{world_,delete_positon_id_});
			die();
		}

		break;

	default:

		origin_position_ = transform_.position();

		break;

	}
}

void SizeBiggerC::draw()const {
	if (is_big_ == true) return;

	glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_LIGHTING);
	glDepthMask(GL_FALSE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glEnable(GL_BLEND);

	GSvector3 sbc_pos_ = origin_position_;
	static const GSrect sbc_rect_{ -image_size_.x / 2.0f,image_size_.y / 2.0f,image_size_.y / 2.0f,-image_size_.x / 2.0f };
	const GSvector2 sbc_scale = { scale_,scale_ };

	gsDrawSprite3D(Image_Gimmick_SizeBiggerC, &sbc_pos_, &sbc_rect_, NULL, NULL, &sbc_scale, 0.0f);
	glPopAttrib();

	//collider().draw();
}

void SizeBiggerC::react(Actor& other) {
	if (other.name() == gimmick_detect_name_)return;

	if (is_big_ == true) return;

	if (other.name() == paddle_name_ && sbc_state_ == SBC_Wait)
	{
		owner_name_ = other.enemy_name();
		sbc_state_ = SBC_Big;
		enable_collider_ = false;
	}
}