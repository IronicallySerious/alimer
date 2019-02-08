/*
** Alimer - Copyright (C) 2017-2019 Amer Koleci.
**
** This file is subject to the terms and conditions defined in
** file 'LICENSE', which is part of this source code package.
*/

#include "ShaderImporter.h"

namespace alimer
{
    ShaderImporter::ShaderImporter(Engine& engine)
        : _engine(engine)
    {

    }

    bool ShaderImporter::IsExtensionSupported(const std::string& extension) const
    {
        return extension == ".hlsl";
    }

    bool ShaderImporter::Import(const std::string& fileName, const std::string& destPath, std::shared_ptr<AssetImporter::Parameters> parameters)
    {
        // Read input
        {
            FileStream source(fileName, FileAccess::ReadOnly);
            std::string shaderSource = source.ReadAllText();
        }

        // Write output
        {
            FileStream output(destPath, FileAccess::WriteOnly);
        }

        return true;
    }
}

