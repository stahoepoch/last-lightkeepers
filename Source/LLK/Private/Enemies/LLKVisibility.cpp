#include "Enemies/LLKVisibility.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"

ELLKVisibility FLLKWorldVisibility::VisibilityBetween(const FVector& From, const FVector& To) const
{
	if (World == nullptr)
	{
		// Not an error and not a clear line: nobody could tell. The caller
		// decides what to do about it, and LLKPerception::Notice decides not
		// to notice.
		return ELLKVisibility::Unknown;
	}

	// Zero-length and near-zero-length queries. A trace between two points
	// one millimetre apart is a trace that always passes, and treating it as
	// Clear would let a test lane of zero length certify a broken trace - see
	// requirement 8 in docs/MAP-READINESS.md, which is why that lane has a
	// minimum length.
	if (FVector::DistSquared(From, To) <= KINDA_SMALL_NUMBER)
	{
		return ELLKVisibility::Clear;
	}

	FCollisionQueryParams Params(SCENE_QUERY_STAT(LLKVisibility), /*bTraceComplex=*/false);
	Params.bReturnPhysicalMaterial = false;
	if (Ignore != nullptr)
	{
		Params.AddIgnoredActor(Ignore);
	}

	// LineTraceTestByChannel rather than Single: nothing here needs the hit
	// location, the material, or which actor stopped it. Perception asks a
	// yes/no question and the cheapest trace that answers it is the one that
	// can run per enemy per keeper.
	const bool bHit = World->LineTraceTestByChannel(From, To, ECC_Visibility, Params);
	return bHit ? ELLKVisibility::Blocked : ELLKVisibility::Clear;
}
