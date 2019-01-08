/*
** Alimer - Copyright (C) 2017-2019 Amer Koleci.
**
** This file is subject to the terms and conditions defined in
** file 'LICENSE', which is part of this source code package.
*/

#pragma once
#include "Alimer.h"
#include "AssetImporter.h"

namespace alimer
{
    class ShaderImporter final : public AssetImporter
    {
    public:
        ShaderImporter(Engine& engine);

        /* AssetImporter members */
        bool IsExtensionSupported(const std::string& extension) const override;
        bool Import(const std::string& fileName, const std::string& destPath, std::shared_ptr<AssetImporter::Parameters> parameters) override;

    private:
        Engine& _engine;
    };
}
