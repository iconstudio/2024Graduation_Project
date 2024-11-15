#include "SagaGummyBearPlayer.h"
#include <Templates/Casts.h>
#include <Engine/TimerHandle.h>
#include <Components/ArrowComponent.h>
#include <Components/SkinnedMeshComponent.h>
#include <GeometryCollection/ManagedArray.h>
#include <GeometryCollection/GeometryCollectionComponent.h>
#include <GeometryCollection/GeometryCollectionObject.h>
#include <GeometryCollection/GeometryCollection.h>
#include <GeometryCollection/GeometryCollectionAlgo.h>
#include <NiagaraComponent.h>
#include <NiagaraFunctionLibrary.h>

#include "SagaGameSubsystem.h"
#include "PlayerControllers/SagaGummyBearController.h"
#include "Player/SagaPlayerTeam.h"
#include "Character/SagaPlayableCharacter.h"
#include "Character/SagaGummyBearAnimInstance.h"
#include "Character/SagaGummyBearSmall.h"
#include "Effect/SagaSwordEffect.h"
#include "UI/SagaWidgetComponent.h"
#include "UI/SagaHpBarWidget.h"

#include "Saga/Network/SagaNetworkSubSystem.h"

float
ASagaGummyBearPlayer::TakeDamage(float dmg, FDamageEvent const& event, AController* instigator, AActor* causer)
{
	const auto net = USagaNetworkSubSystem::GetSubSystem(GetWorld());

	if (net->IsOfflineMode())
	{
#if WITH_EDITOR

		const auto name = GetName();
		UE_LOG(LogSagaGame, Log, TEXT("[TakeDamage] Guardian '%s' would take %f damages. (Offline Mode)"), *name, dmg);
#endif

		return Super::TakeDamage(dmg, event, instigator, causer);
	}
	else if (IsAlive())
	{
#if WITH_EDITOR

		UE_LOG(LogSagaGame, Log, TEXT("[TakeDamage] Handling gummy bear's damage by %f."), dmg);
#endif

		// 서버의 RPC_DMG_PLYER 처리 부분의 주석 참조
		// arg0: 플레이어가 준 피해량 (4바이트 부동소수점) | 파괴 부위 (4바이트)
		int64 arg0{};
		// arg1: 현재 수호자의 식별자
		int32 arg1{ GetBearId() };

		std::memcpy(&arg0, reinterpret_cast<const char*>(&dmg), 4);

		net->SendRpcPacket(ESagaRpcProtocol::RPC_DMG_GUARDIAN, arg0, arg1);
	}

	return dmg;
}

void
ASagaGummyBearPlayer::ExecuteGuardianAction(ASagaCharacterBase* target)
{
	Super::ExecuteGuardianAction(target);
	SetTeam(target->GetTeam());

	const auto healthbar = Cast<USagaHpBarWidget>(healthIndicatorBarWidget->GetWidget());

	if (IsValid(healthbar))
	{
		healthbar->UpdateHpBar(myGameStat->GetCurrentHp() / myGameStat->GetMaxHp());
	}

	ownerData.SetCharacterHandle(target);
}

void
ASagaGummyBearPlayer::TerminateGuardianAction()
{
	Super::TerminateGuardianAction();
	SetTeam(ESagaPlayerTeam::Unknown);

	ChangeColliderProfileToHostile();

	const auto character = ownerData.GetCharacterHandle();

	if (IsValid(character))
	{
		const auto loc = playerUnridePosition->GetComponentLocation();
		const auto rot = playerUnridePosition->GetComponentRotation();
		character->SetActorLocationAndRotation(loc, rot, false, nullptr, ETeleportType::ResetPhysics);

		ResetOwnerData();
	}
	else
	{
#if WITH_EDITOR

		UE_LOG(LogSagaGame, Error, TEXT("[TerminateGuardianAction] Has no character handle!"));
#endif
	}

	myController->Possess(this);
}

void
ASagaGummyBearPlayer::ExecuteAttackAnimation()
{
	if (IsValid(mBearAnimInst))
	{
		mBearAnimInst->PlayAttackMontage();
	}
	else
	{
#if WITH_EDITOR

		UE_LOG(LogSagaGame, Error, TEXT("[ASagaGummyBearPlayer] mBearAnimInst is null."));
#endif
	}
}

void
ASagaGummyBearPlayer::ExecuteAttack()
{
#if WITH_EDITOR

	const auto name = GetName();
#endif

	const auto net = USagaNetworkSubSystem::GetSubSystem(GetWorld());
	if (net->GetLocalUserId() != GetUserId())
	{
#if WITH_EDITOR

		//UE_LOG(LogSagaGame, Warning, TEXT("[ASagaGummyBearPlayer] '%s' is not local character. (net id: %d, entity id: %d)"), *name, net->GetLocalUserId(), GetUserId());
#endif

		// NOTICE: 이거 하면 데미지 처리 안됨
		//return;
	}

	//공격과 충돌되는 물체 여부 판단
	FHitResult hit_result{};

	FVector Start = GetActorLocation() + GetActorForwardVector() * 50.f;
	FVector End = Start + GetActorForwardVector() * 150.f;

	FCollisionQueryParams param = FCollisionQueryParams::DefaultQueryParam;
	param.AddIgnoredActor(this);

	///NOTICE: ECC_GameTraceChannel2: Enemy
	///NOTICE: ECC_GameTraceChannel4: red
	///NOTICE: ECC_GameTraceChannel7: blue
	ECollisionChannel channel = ECC_GameTraceChannel2;

	if (GetTeam() == ESagaPlayerTeam::Red)
	{
		channel = ECC_GameTraceChannel4;
	}
	else if (GetTeam() == ESagaPlayerTeam::Blue)
	{
		channel = ECC_GameTraceChannel7;
	}
	else
	{
#if WITH_EDITOR

		UE_LOG(LogSagaGame, Error, TEXT("[ASagaGummyBearPlayer] '%s' has invalid team."), *name);
#endif
	}

	const bool collide = GetWorld()->SweepSingleByChannel(hit_result, Start, End, FQuat::Identity, channel, FCollisionShape::MakeSphere(50.f), param);

	//#if ENABLE_DRAW_DEBUG
	//	//충돌시 빨강 아니면 녹색
	//	FColor Color = collide ? FColor::Red : FColor::Green;
	//
	//	DrawDebugCapsule(GetWorld(), (Start + End) / 2.f, 150.f / 2.f + 50.f / 2.f, 50.f, FRotationMatrix::MakeFromZ(GetActorForwardVector()).ToQuat(), Color, false, 3.f);
	//#endif

	constexpr float bearDamage = 50.0f;

	if (collide)
	{
		const auto hit_actor = hit_result.GetActor();

		FDamageEvent event{};
		FActorSpawnParameters params{};
		params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		const auto character = Cast<ASagaCharacterBase>(hit_actor);

		if (IsValid(character))
		{
			hit_actor->TakeDamage(bearDamage, event, GetController(), this);

			const auto hit_effect = GetWorld()->SpawnActor<ASagaSwordEffect>(hit_result.ImpactPoint, hit_result.ImpactNormal.Rotation(), params);

			// 이곳에 레퍼런스 복사
			//hit_effect->SetParticle(TEXT(""));
			//hit_effect->SetSound(TEXT(""));
		}
		else
		{
			const auto ai_pawn = Cast<ASagaGummyBearSmall>(hit_actor);

			if (IsValid(ai_pawn))
			{
				hit_actor->TakeDamage(bearDamage, event, GetController(), this);

				const auto hit_effect = GetWorld()->SpawnActor<ASagaSwordEffect>(hit_result.ImpactPoint, hit_result.ImpactNormal.Rotation(), params);

				// 이곳에 레퍼런스 복사
				//hit_effect->SetParticle(TEXT(""));
				//hit_effect->SetSound(TEXT(""));
			}
			else
			{
				// 아이템 혹은 다른 액터 공격
				hit_actor->TakeDamage(bearDamage, event, GetController(), this);

#if WITH_EDITOR

				UE_LOG(LogSagaGame, Log, TEXT("[ASagaGummyBearPlayer] '%s' attacked other actors."), *name);
#endif
			}
		}
	}
}

float
ASagaGummyBearPlayer::ExecuteHurt(const float dmg)
{
	const auto current_hp = Super::ExecuteHurt(dmg);

#if WITH_EDITOR

	UE_LOG(LogSagaGame, Log, TEXT("[ASagaGummyBearPlayer] ExecuteHurt (dmg: %f, hp : %f)"), dmg, current_hp);
#endif

	// 
	if (HitEffect)
	{
		FVector SpawnLocation = GetActorLocation();
		FRotator SpawnRotation = GetActorRotation();
		UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, HitEffect, SpawnLocation, SpawnRotation);

		if (NiagaraComponent)
		{
			FTimerHandle NiagaraTimerHandle{};

			GetWorldTimerManager().SetTimer(NiagaraTimerHandle
				, [NiagaraComponent]()
				{
					NiagaraComponent->Deactivate();
					NiagaraComponent->DestroyComponent();
				}, 3.0f, false
			);
		}
	}

	return current_hp;
}

void
ASagaGummyBearPlayer::ExecuteDeath()
{
	// 사망 처리
	UE_LOG(LogSagaGame, Log, TEXT("[ASagaGummyBearPlayer] ExecuteDeath"));

	// 사망 애니메이션 실행
	mBearAnimInst->Death();

	const auto world = GetWorld();
	const auto net = USagaNetworkSubSystem::GetSubSystem(world);

	if (net->IsOfflineMode())
	{
		const auto sys = USagaGameSubsystem::GetSubSystem(world);
		Super::ExecuteDeath();

		// 상대 팀 점수 증가 실행
		const auto team = GetTeam();
		if (team == ESagaPlayerTeam::Red)
		{
			sys->AddScore(ESagaPlayerTeam::Blue, 1);
		}
		else if (team == ESagaPlayerTeam::Blue)
		{
			sys->AddScore(ESagaPlayerTeam::Red, 1);
		}
	}
	else
	{
		Super::ExecuteDeath();

		net->SendRpcPacket(ESagaRpcProtocol::RPC_GET_SCORE);
	}
}

UGeometryCollectionComponent*
ASagaGummyBearPlayer::ExecutePartDestruction(const int32 index)
{
	const auto rot = GetActorRotation() + FRotator{ 0, 0, 180 };
	const FVector Impulse = rot.Vector() * 200;

	USkinnedMeshComponent* const SkinnedMesh = FindComponentByClass<USkinnedMeshComponent>();
	if (!IsValid(SkinnedMesh))
	{
		UE_LOG(LogSagaGame, Error, TEXT("[ExecutePartDestruction] There is no USkinnedMeshComponent."));

		return nullptr;
	}

	if (!DismCollisionBox.IsValidIndex(index))
	{
#if WITH_EDITOR

		UE_LOG(LogSagaGame, Error, TEXT("[ExecutePartDestruction] %d is an invalid index for colliders."), index);
#endif

		return nullptr;
	}

	const auto& body_collider = DismCollisionBox[index];

	const FName bone_name = body_collider->GetAttachSocketName();
	if (!SkinnedMesh->IsBoneHiddenByName(bone_name))
	{
#if WITH_EDITOR

		UE_LOG(LogSagaGame, Warning, TEXT("[ExecutePartDestruction] TriggerExplosion: %d"), index);
#endif

		SkinnedMesh->HideBoneByName(bone_name, PBO_None);

		const auto& gc = GeometryCollections[index];
		gc->SetVisibility(true);
		gc->SetSimulatePhysics(true);
		gc->CrumbleActiveClusters();

		gc->AddRadialImpulse(gc->GetComponentLocation(), 100, 150, RIF_Constant, true);
		gc->AddImpulse(Impulse, NAME_None, true);

		const auto world = GetWorld();
		const auto net = USagaNetworkSubSystem::GetSubSystem(world);

		// 사망 처리
		if (net->IsOfflineMode())
		{
			const auto& right_leg_hp = ActiveIndex[1];
			const auto& left_leg_hp = ActiveIndex[3];

			if (right_leg_hp <= 0 && left_leg_hp <= 0)
			{
#if WITH_EDITOR

				const auto name = GetName();

				UE_LOG(LogSagaGame, Log, TEXT("[ASagaPlayableCharacter][Attack] '%s' is dead because all legs are destructed. (Offline Mode)"), *name);
				//UE_LOG(LogSagaGame, Log, TEXT("[ASagaPlayableCharacter][Attack] '%s' is dead because all legs are destructed."), *name);
#endif

				ExecuteHurt(GetHealth());
			}
		}

		return gc;
	}
	else
	{
		return nullptr;
	}
}

void
ASagaGummyBearPlayer::ExecuteMorphingPart(UGeometryCollectionComponent* const& gc, const int32 index)
{
	//DismCollisionBox[Index]->DestroyComponent();
	//SpawnMorphSystem(TargetGC, Index);

	if (UFunction* morph_fun = FindFunction(TEXT("TriggerMorphEvent")))
	{
		struct Params
		{
			UGeometryCollectionComponent* TargetGC;
			int32 Index;
		};

		Params parameters{};
		parameters.TargetGC = gc;
		parameters.Index = index;
		ProcessEvent(morph_fun, &parameters);
	}
}

void
ASagaGummyBearPlayer::ExecuteMorphingPartAt(const int32 index)
{
	const auto& gc = GeometryCollections[index];

	ExecuteMorphingPart(gc, index);
}

void
ASagaGummyBearPlayer::RegisterMiniBear(const int32 index, ASagaGummyBearSmall* const minibear)
{
	partedSmallBears[index] = minibear;
}

int32
ASagaGummyBearPlayer::OnBodyPartGetDamaged(FVector Location, FVector Normal)
{
	const auto world = GetWorld();
	const auto net = USagaNetworkSubSystem::GetSubSystem(world);

	if (IsAlive())
	{
		for (int32 i = 0; i < DismCollisionBox.Num(); i++)
		{
			if (IsPointInsideBox(DismCollisionBox[i], Location))
			{
				DismPartID = i;

				if (net->IsOfflineMode())
				{
					if (GiveDamageToPart(i))
					{
#if WITH_EDITOR

						const auto name = GetName();

						UE_LOG(LogSagaGame, Log, TEXT("[OnBodyPartGetDamaged] A part %d of '%s' is destructed. (Offline Mode)"), i, *name);
#endif

						const auto part_actor = ExecutePartDestruction(i);

						if (part_actor)
						{
							ExecuteMorphingPart(part_actor, i);
						}

						return i;
					}
				}
				else if (not IsPartDestructed(i))
				{
					// RPC 송신
					return i;
				}
			}

			//UE_LOG(LogSagaGame, Log, TEXT("[ASagaGummyBearPlayer] HitBox: %d, hp: %d"), i, ActiveIndex[i]);
		}
	}

	return -1;
}

bool
ASagaGummyBearPlayer::GiveDamageToPart(const int32 index, const int32 part_dmg)
{
	auto& part_hp = ActiveIndex[index];

	if (--part_hp == 0)
	{
		//UE_LOG(LogSagaGame, Warning, TEXT("HitBox: %d, boxHP: %d"), i, ActiveIndx[i]);
		return true;
	}
	else
	{
		return false;
	}
}

bool
ASagaGummyBearPlayer::IsPartDestructed(const int32 index)
const
{
	return ActiveIndex[index] <= 0;
}

FTransform
ASagaGummyBearPlayer::SpawnMorphSystem(UGeometryCollectionComponent* TargetGC, int32 Index)
{
	TargetGC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	int32 PieceIndex = 0;

	switch (Index)
	{
	case 0:
		PieceIndex = 13;
		break;
	case 1:
		PieceIndex = 10;
		break;
	case 2:
		PieceIndex = 6;
		break;
	case 3:
		PieceIndex = 4;
		break;
	}

	FVector Location;
	FQuat Rotation;

	auto& transforms = TargetGC->GetTransformArray();
	auto& gc_transform = TargetGC->GetComponentTransform();

	if (transforms.IsValidIndex(PieceIndex))
	{
		FTransform WorldTransform = transforms[PieceIndex] * gc_transform;

		Location = WorldTransform.GetLocation();
		Rotation = WorldTransform.GetRotation();
	}
	else
	{
		UE_LOG(LogSagaGame, Fatal, TEXT("[SpawnMorphSystem] There is no transform of the piece %d."), Index);
	}

	if (NiagaraSystemTemplate)
	{
		FHitResult hitresult{};
		FVector Starttrace = Location + FVector(0, 0, 100);
		FVector Endtrace = Location + FVector(0, 0, -1000);

		const auto world = GetWorld();

		if (world->LineTraceSingleByChannel(hitresult, Starttrace, Endtrace, ECollisionChannel::ECC_Visibility))
		{
			NiagaraComponentTemplate->SetAsset(NiagaraSystemTemplate);
			Location = hitresult.Location + FVector(0, 0, 45.0f);

			UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				world,
				NiagaraSystemTemplate,
				Location,
				FRotator::ZeroRotator,
				FVector(1.f)
			);

			UNiagaraFunctionLibrary::OverrideSystemUserVariableStaticMesh(NiagaraComponent, TEXT("User.SourceMesh"), nullptr);
			UNiagaraFunctionLibrary::OverrideSystemUserVariableStaticMesh(NiagaraComponent, TEXT("User.SourceMesh"), GetTargetMesh(Index));

			NiagaraComponent->SetVariableQuat(TEXT("InitRotator"), Rotation);
			NiagaraComponent->ActivateSystem(true);

			HideBodyPartPiece(TargetGC, PieceIndex);

			FTransform output{};
			output.SetLocation(Location);
			output.SetRotation(FQuat::Identity);

			return output;
		}

	}

	return FTransform::Identity;
}

UStaticMesh*
ASagaGummyBearPlayer::GetTargetMesh(int32 Index)
{
	return TargetMeshes[Index];
}

void
ASagaGummyBearPlayer::InitTargetMeshes()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh1Asset(TEXT("StaticMesh'/Game/NPCAssets/meshes/rarm_13.rarm_13'"));
	if (Mesh1Asset.Succeeded())
	{
		TargetMeshes.Add(Mesh1Asset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh2Asset(TEXT("StaticMesh'/Game/NPCAssets/meshes/rleg_10.rleg_10'"));
	if (Mesh2Asset.Succeeded())
	{
		TargetMeshes.Add(Mesh2Asset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh3Asset(TEXT("StaticMesh'/Game/NPCAssets/meshes/larm_6.larm_6'"));
	if (Mesh3Asset.Succeeded())
	{
		TargetMeshes.Add(Mesh3Asset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh4Asset(TEXT("StaticMesh'/Game/NPCAssets/meshes/lleg_4.lleg_4'"));
	if (Mesh3Asset.Succeeded())
	{
		TargetMeshes.Add(Mesh4Asset.Object);
	}

	//UE_LOG(LogSagaGame, Warning, TEXT("targetmesheslen: %d"), TargetMeshes.Num());
}

ASagaCharacterBase*
ASagaGummyBearPlayer::GetStoredCharacterHandle()
const noexcept
{
	return ownerData.GetCharacterHandle();
}

int32
ASagaGummyBearPlayer::GetBearId()
const noexcept
{
	return bearUniqueId;
}

ASagaGummyBearPlayer::ASagaGummyBearPlayer()
	: Super()
	, bearUniqueId(0)
	, playerUnridePosition()
	, myInteractionBox()
{
	myGameStat->SetMaxHp(500.0f);

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(TEXT("/Script/Engine.SkeletalMesh'/Game/NPCAssets/Modeling/Bear.Bear'"));
	if (MeshAsset.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshAsset.Object);
	}

	GetCapsuleComponent()->SetCapsuleSize(90.0f, 96.0f);
	SetRootComponent(GetCapsuleComponent());
	ChangeColliderProfileToHostile();

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetRelativeLocation(FVector(0.0, 0.0, -88.0));
	GetMesh()->SetRelativeRotation(FRotator(0.0, -90.0, 0.0));

	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimAsset(TEXT("AnimBlueprint'/Game/NPCAssets/Animation/AB_GummyBear.AB_GummyBear_C'"));
	if (AnimAsset.Succeeded())
	{
		GetMesh()->SetAnimInstanceClass(AnimAsset.Class);
	}

	playerUnridePosition = CreateDefaultSubobject<UArrowComponent>(TEXT("PlayerUnridePosition"));
	playerUnridePosition->SetupAttachment(RootComponent);
	playerUnridePosition->SetRelativeLocation(FVector(-170, 0, 110.0));
	playerUnridePosition->SetRelativeRotation(FRotator(0, 0, 0));

	myCameraSpringArmComponent->SetRelativeLocation(FVector(0.0, 0.0, 250.0));
	myCameraSpringArmComponent->SetRelativeRotation(FRotator(-30.0, 90.0, 0.0));
	myCameraSpringArmComponent->TargetArmLength = 400.f;

	myInteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	myInteractionBox->SetupAttachment(RootComponent);
	myInteractionBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	myInteractionBox->SetWorldScale3D(FVector(4.5f, 5.75f, 2.25f));
	myInteractionBox->SetRelativeLocation(FVector(70.0f, 0.0f, 40.0f));

	//*/
	Dbox_Rarm = CreateDefaultSubobject<UBoxComponent>(TEXT("Dbox_Rarm"));
	Dbox_Rarm->SetupAttachment(GetMesh(), TEXT("arm_stretch_r"));
	InitializeTransform(Dbox_Rarm, FVector(-50, -65, -7), FRotator(6, -50, 82), FVector(1.25, 1.25, 2.25));
	Dbox_Rarm->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DismCollisionBox.Add(Dbox_Rarm);

	Dbox_Rleg = CreateDefaultSubobject<UBoxComponent>(TEXT("Dbox_Rleg"));
	Dbox_Rleg->SetupAttachment(GetMesh(), TEXT("thigh_stretch_r"));
	InitializeTransform(Dbox_Rleg, FVector(2, -25, 10), FRotator(-30, 57, 72), FVector(1.5, 1.5, 2.0));
	Dbox_Rleg->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DismCollisionBox.Add(Dbox_Rleg);

	Dbox_Larm = CreateDefaultSubobject<UBoxComponent>(TEXT("Dbox_Larm"));
	Dbox_Larm->SetupAttachment(GetMesh(), TEXT("arm_stretch_l"));
	InitializeTransform(Dbox_Larm, FVector(40, -80, -15), FRotator(6, 40, 76), FVector(1.25, 1.25, 2.0));
	Dbox_Larm->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DismCollisionBox.Add(Dbox_Larm);

	Dbox_Lleg = CreateDefaultSubobject<UBoxComponent>(TEXT("Dbox_Lleg"));
	Dbox_Lleg->SetupAttachment(GetMesh(), TEXT("thigh_stretch_l"));
	InitializeTransform(Dbox_Lleg, FVector(-8, -15, 8), FRotator(-50, 100, -45), FVector(1.25, 1.25, 2.0));
	Dbox_Lleg->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DismCollisionBox.Add(Dbox_Lleg);

	//*/
	r_arm = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("r_arm"));
	r_arm->SetupAttachment(Dbox_Rarm);
	static ConstructorHelpers::FObjectFinder<UGeometryCollection> r_armAsset(TEXT("GeometryCollection'/Game/NPCAssets/meshes/GC_r_arm.GC_r_arm'"));
	if (r_armAsset.Succeeded())
	{
		r_arm->RestCollection = r_armAsset.Object;
	}
	InitializeTransform(r_arm, FVector(2, 2, 6), FRotator(80, 68, -50), FVector(0.8, 0.8, 0.44));
	GeometryCollections.Add(r_arm);

	r_leg = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("r_leg"));
	r_leg->SetupAttachment(Dbox_Rleg);
	static ConstructorHelpers::FObjectFinder<UGeometryCollection> r_legAsset(TEXT("GeometryCollection'/Game/NPCAssets/meshes/GC_r_leg.GC_r_leg'"));
	if (r_legAsset.Succeeded())
	{
		r_leg->RestCollection = r_legAsset.Object;
	}
	InitializeTransform(r_leg, FVector(3.8, -3, -10), FRotator(-5, 41, 0), FVector(0.66, 0.66, 0.5));
	GeometryCollections.Add(r_leg);

	l_arm = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("l_arm"));
	l_arm->SetupAttachment(Dbox_Larm);
	static ConstructorHelpers::FObjectFinder<UGeometryCollection> l_armAsset(TEXT("GeometryCollection'/Game/NPCAssets/meshes/GC_l_arm.GC_l_arm'"));
	if (l_armAsset.Succeeded())
	{
		l_arm->RestCollection = l_armAsset.Object;
	}
	InitializeTransform(l_arm, FVector(1, 3, 8), FRotator(-82, -141, 54), FVector(0.8, 0.8, 0.5));
	GeometryCollections.Add(l_arm);

	l_leg = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("l_leg"));
	l_leg->SetupAttachment(Dbox_Lleg);
	static ConstructorHelpers::FObjectFinder<UGeometryCollection> l_legAsset(TEXT("GeometryCollection'/Game/NPCAssets/meshes/GC_l_leg.GC_l_leg'"));
	if (l_legAsset.Succeeded())
	{
		l_leg->RestCollection = l_legAsset.Object;
	}
	InitializeTransform(l_leg, FVector(0, 2, -12), FRotator(2, 154, 2), FVector(0.8, 0.66, 0.5));
	GeometryCollections.Add(l_leg);

	for (int i = 0; i < GeometryCollections.Num(); i++)
	{
		GeometryCollections[i]->SetVisibility(false);
		GeometryCollections[i]->SetSimulatePhysics(false);
		GeometryCollections[i]->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	}

	partedSmallBears.Reserve(4);
	for (int32 i = 0; i < 4; ++i)
	{
		partedSmallBears.AddDefaulted();
	}

	InitTargetMeshes();

	//*/
	DismPartID = -1;
	DismThreshold = 1;

	NiagaraComponentTemplate = CreateDefaultSubobject<UNiagaraComponent>(TEXT("MorphNiagaraComponent"));
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> NSAsset(TEXT("NiagaraSystem'/Game/NPCAssets/NS_MeshMorph.NS_MeshMorph'"));
	if (NSAsset.Succeeded())
	{
		NiagaraSystemTemplate = NSAsset.Object;
	}
	NiagaraComponentTemplate->SetupAttachment(RootComponent);
	NiagaraComponentTemplate->SetAutoActivate(false);

	// 오버랩 이벤트 바인드
	//myInteractionBox->OnComponentBeginOverlap.AddDynamic(this, &ASagaGummyBearPlayer::OnOverlapBegin);
	//myInteractionBox->OnComponentEndOverlap.AddDynamic(this, &ASagaGummyBearPlayer::OnOverlapEnd);

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> NiagaraEffect(TEXT("/Script/Niagara.NiagaraSystem'/Game/VFX/VFX_Hit/NS_Hit.NS_Hit'"));
	if (NiagaraEffect.Succeeded())
	{
		HitEffect = NiagaraEffect.Object;
		UE_LOG(LogTemp, Warning, TEXT("Bear Niagara Hit Effect Loaded"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Bear Item Niagara Hit Effect Not Loaded"));
	}
}

void
ASagaGummyBearPlayer::BeginPlay()
{
	myGameStat->OnHpZero.AddDynamic(this, &ASagaGummyBearPlayer::ExecuteDeath);

	ActiveIndex.Init(DismThreshold, 4);

	Super::BeginPlay();
}

void
ASagaGummyBearPlayer::InitializeTransform(USceneComponent* const component, const FVector& loc, const FRotator& rot, const FVector& scale)
{
	component->SetRelativeLocation(loc);
	component->SetRelativeRotation(rot);
	component->SetRelativeScale3D(scale);
}

bool
ASagaGummyBearPlayer::IsPointInsideBox(const UBoxComponent* const Box, const FVector& Point)
{
	if (!Box) return false;

	FVector BoxOrigin = Box->GetComponentLocation();
	FVector BoxExtent = Box->GetScaledBoxExtent();
	FRotator BoxRotation = Box->GetComponentRotation();
	FTransform BoxTransform(BoxRotation, BoxOrigin);

	FVector LocalPoint = BoxTransform.InverseTransformPosition(Point);

	return FMath::Abs(LocalPoint.X) <= BoxExtent.X &&
		FMath::Abs(LocalPoint.Y) <= BoxExtent.Y &&
		FMath::Abs(LocalPoint.Z) <= BoxExtent.Z;
}

void
ASagaGummyBearPlayer::HideBodyPartPiece(UGeometryCollectionComponent* const TargetGC, int32 PieceIndex)
{
	TargetGC->SetVisibility(false, true);
}

UGeometryCollectionComponent*
ASagaGummyBearPlayer::FindGeometryCollectionByName(const AActor* const actor, const FString& name)
{
	auto& owner = actor->Owner;

	if (IsValid(owner))
	{
		TArray<UActorComponent*> Components;
		owner->GetComponents(Components);

		for (const auto Component : Components)
		{
			const auto GeometryCollectionComponent = Cast<UGeometryCollectionComponent>(Component);

			if (GeometryCollectionComponent && GeometryCollectionComponent->GetName() == name)
			{
				return GeometryCollectionComponent;
			}
		}
	}

	return nullptr;
}

FVector
ASagaGummyBearPlayer::GetPieceWorldLocation(const UGeometryCollectionComponent* const target, int32 index)
{
	if (target)
	{
		const TManagedArray<FTransform>& Transforms = target->GetTransformArray();

		if (Transforms.IsValidIndex(index))
		{
			FTransform WorldTransform = Transforms[index] * target->GetComponentTransform();

			return WorldTransform.GetLocation();
		}
	}

	return FVector::ZeroVector;
}

FQuat
ASagaGummyBearPlayer::GetPieceWorldRotation(const UGeometryCollectionComponent* const target, int32 index)
{
	if (target)
	{
		const UGeometryCollection* GeometryCollection = target->GetRestCollection();
		FGeometryCollection* Collection = GeometryCollection->GetGeometryCollection().Get();
		FTransform Transform = Collection->Transform[index];

		const TManagedArray<FTransform>& Transforms = target->GetTransformArray();

		if (Transforms.IsValidIndex(index))
		{
			FTransform WorldTransform = Transform * target->GetComponentTransform();

			return WorldTransform.GetRotation();
		}
	}

	return FQuat::Identity;
}
