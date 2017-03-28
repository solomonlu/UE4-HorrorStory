//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "MathUtils.generated.h"

USTRUCT(Blueprintable)
struct DUNGEONARCHITECTRUNTIME_API FRectangle {
	GENERATED_USTRUCT_BODY()

		FRectangle() {
		Location = FIntVector(0, 0, 0);
		Size = FIntVector(0, 0, 0);
	}

	FRectangle(int32 x, int32 y, int32 width, int32 height) {
		Location = FIntVector(x, y, 0);
		Size = FIntVector(width, height, 0);
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
	FIntVector Location;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
	FIntVector Size;

	FORCEINLINE int32 X() const { return Location.X; }
	FORCEINLINE int32 Y() const { return Location.Y; }
	FORCEINLINE int32 Width() const { return Size.X; }
	FORCEINLINE int32 Height() const { return Size.Y; }
	FORCEINLINE FIntVector Center() const { return Location + Size / 2; }

	void Clip(const FRectangle& ClipBounds) {
		int32 X0 = Location.X;
		int32 X1 = Location.X + Size.X;
		int32 Y0 = Location.Y;
		int32 Y1 = Location.Y + Size.Y;
		int32 Z0 = Location.Z;
		int32 Z1 = Location.Z + Size.Z;

		int32 CX0 = ClipBounds.Location.X;
		int32 CX1 = ClipBounds.Location.X + ClipBounds.Size.X;
		int32 CY0 = ClipBounds.Location.Y;
		int32 CY1 = ClipBounds.Location.Y + ClipBounds.Size.Y;
		int32 CZ0 = ClipBounds.Location.Z;
		int32 CZ1 = ClipBounds.Location.Z + ClipBounds.Size.Z;

		X0 = FMath::Clamp(X0, CX0, CX1);
		X1 = FMath::Clamp(X1, CX0, CX1);
		Y0 = FMath::Clamp(Y0, CY0, CY1);
		Y1 = FMath::Clamp(Y1, CY0, CY1);
		Z0 = FMath::Clamp(Z0, CZ0, CZ1);
		Z1 = FMath::Clamp(Z1, CZ0, CZ1);

		Location = FIntVector(X0, Y0, Z0);
		Size = FIntVector(X1 - X0, Y1 - Y0, Z1 - Z0);
	}

	bool Contains(const FRectangle& rect) const {
		return(X() <= rect.X()) &&
			((rect.X() + rect.Width()) <= (X() + Width())) &&
			(Y() <= rect.Y()) &&
			((rect.Y() + rect.Height()) <= (Y() + Height()));
	}

	bool Contains(const FIntVector& Point) const {
		return Contains(Point.X, Point.Y);
	}

	bool Contains(int x, int y) const {
		return X() <= x &&
			x < X() + Width() &&
			Y() <= y &&
			y < Y() + Height();
	}

	static FRectangle Intersect(const FRectangle& a, const FRectangle& b) {
		int x1 = FMath::Max(a.X(), b.X());
		int x2 = FMath::Min(a.X() + a.Width(), b.X() + b.Width());
		int y1 = FMath::Max(a.Y(), b.Y());
		int y2 = FMath::Min(a.Y() + a.Height(), b.Y() + b.Height());

		if (x2 >= x1 && y2 >= y1) {
			return FRectangle(x1, y1, x2 - x1, y2 - y1);
		}
		return FRectangle();
	}

	bool IntersectsWith(const FRectangle& rect) const {
		return(rect.X() < X() + Width()) &&
			(X() < (rect.X() + rect.Width())) &&
			(rect.Y() < Y() + Height()) &&
			(Y() < rect.Y() + rect.Height());
	}
};


class DUNGEONARCHITECTRUNTIME_API FMathUtils {
public:
	static FVector ToVector(const FIntVector& value) {
		return FVector(value.X, value.Y, value.Z);
	}
	static FIntVector ToIntVector(const FVector& value, bool bRound = false) {
		if (bRound) {
			return FIntVector(
				FMath::RoundToInt(value.X), 
				FMath::RoundToInt(value.Y),
				FMath::RoundToInt(value.Z));
		}
		else {
			return FIntVector(
				FMath::FloorToInt(value.X),
				FMath::FloorToInt(value.Y),
				FMath::FloorToInt(value.Z));
		}
	}

	static FVector2D GetRandomDirection2D(FRandomStream& Random) {
		float Angle = Random.FRand() * PI * 2;
		return FVector2D(FMath::Cos(Angle), FMath::Sin(Angle));
	}

	static TArray<int32> GetShuffledIndices(int32 Count, FRandomStream& Random);
	
	template<typename T>
	static T GetRandomItem(TArray<T>& Array, FRandomStream& Random) {
		return Array[Random.RandRange(0, Array.Num() - 1)];
	}

	template<typename T> 
	static void Shuffle(TArray<T>& Array, FRandomStream& Random) {
		int32 Count = Array.Num();
		for (int i = 0; i < Count; i++) {
			int32 j = Random.RandRange(0, Count - 1);
			T Temp = Array[i];
			Array[i] = Array[j];
			Array[j] = Temp;
		}
	}
};