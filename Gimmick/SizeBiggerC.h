#ifndef SIZE_BIGGER_C_H_
#define SIZE_BIGGER_C_H_

#include"Actor/Actor.h"
#include"Util/Util.h"
#include"Util/UtilNumber.h"

class SizeBiggerC :public Actor {

public:

	SizeBiggerC(IWorld* world=nullptr, const GSvector3& position={0.0f,0.0f,0.0f}, const int delete_num = -1);
	void update(float delta_time)override;
	void draw()const override;
	virtual void react(Actor& other)override;

private:

	int sbc_state_{ 0 };

	int p1_flg_{ 0 };

	int p2_flg_{ 0 };

	int cpu_flg_{ 0 };

	float sbc_radius_{ 0.0f };	//半径

	float big_time{ 0.0f };    //大きくなる時のタイマー
	float small_time{ 0.0f };  //小さくなる時のタイマー
	float bigger_time{ 0.0f }; //大きさ維持時タイマー

	bool is_big_{ false }; //bigステートになったか

	GSvector3 reflesh_speed_{ 0.0f,0.0f,0.0f };//アクターの数字を戻す用の変数
	GSvector3 origin_speed_{ 0.0f,0.0f,0.0f };//触れたアクターの元のスピード
	GSvector3 origin_position_{ 0.0f,0.0f,0.0f };//他アクターに触れる前のプラズマグネットの座標
	string other_tag_;

	GSvector2 image_size_{ 0.0f,0.0f };	//画像サイズ

	Util util_;

	enum SBCState {
		SBC_Wait,
		SBC_Bigger,
		SBC_Big,
		SBC_smaller,
	};

};

#endif