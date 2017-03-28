//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//


#pragma once

#include "IDetailCustomization.h"
#include "IPropertyTypeCustomization.h"

class FSnapDoorMeshInfoCustomization : public IDetailCustomization
{
public:

	// IDetailCustomization interface
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	// End of IDetailCustomization interface

	static TSharedRef<IDetailCustomization> MakeInstance();

};
