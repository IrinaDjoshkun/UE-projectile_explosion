// Copyright Epic Games, Inc. All Rights Reserved.

#include "testtaskProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"

AtesttaskProjectile::AtesttaskProjectile() 
{

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &AtesttaskProjectile::OnHit);


	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;


	RootComponent = CollisionComp;


	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	InitialLifeSpan = 3.0f;

	bParameterEnabled = false; //Task#2. Default value of parameter is off 
}

void AtesttaskProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr) && OtherComp->IsSimulatingPhysics())
	{
		Explode();
		DamageCheck();
		OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());
		UPrimitiveComponent* mesh = Cast<UPrimitiveComponent>(OtherActor->GetRootComponent());
		if (mesh)
		{
			if (mesh->IsSimulatingPhysics())
			{
				FVector forceVector = OtherActor->GetActorLocation() - GetActorLocation();
				forceVector.Normalize();
				mesh->AddForce(forceVector * 1000000 * mesh->GetMass()); // Task#1. AddForce function.
				
			}
		}
		Destroy();
	}
}

//Task#2. Explosion method if parameter is on
void AtesttaskProjectile::Explode()
{
	if (bParameterEnabled != false)
	{
		TArray<FHitResult> OutHits;
		FVector startPos = GetActorLocation();
		FVector endPos = startPos + FVector(0.1f);
		FCollisionShape Shape = FCollisionShape::MakeSphere(500.f);
		FCollisionQueryParams params = FCollisionQueryParams::DefaultQueryParam;
		params.AddIgnoredActor(this);
		params.bTraceComplex = true;
		params.TraceTag = "Explode Trace";
		FQuat Rotation = FQuat::Identity;
		bool isHit = GetWorld()->SweepMultiByChannel(OutHits, startPos, endPos, Rotation, ECollisionChannel::ECC_Visibility, Shape, params);
		GetWorld()->DebugDrawTraceTag = "Explode Trace";
		if (isHit)
		{
			for (auto& Hit : OutHits)
			{
				UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>((Hit.GetActor())->GetRootComponent());
				if (MeshComp)
				{
					MeshComp->AddRadialImpulse(startPos, 500.f, 2000.f, ERadialImpulseFalloff::RIF_Constant, true);
				}

			}
		}
	}
}

//Task#2. Damage check method
void AtesttaskProjectile::DamageCheck()
{
	AActor* OtherActor = nullptr;
	AActor* owner = GetOwner();
	AActor* ownerByOwner = owner != nullptr ? owner->GetOwner() : nullptr;

	if (OtherActor != owner && OtherActor != ownerByOwner)
	{
		IDamageTaker* damageTakerActor = Cast<IDamageTaker>(OtherActor);
		if (damageTakerActor)
		{
			FDamageData damageData;
			damageData.DamageValue = Damage;
			damageData.Instigator = owner;
			damageData.DamageMaker = this;
			damageTakerActor->TakeDamage(damageData);
		}
	}
}



