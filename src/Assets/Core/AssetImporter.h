//
// Copyright (c) 2017-2019 Amer Koleci and contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include "Alimer.h"

namespace alimer
{
	class AssetImporter : public Object
	{
		ALIMER_OBJECT(AssetImporter, Object);
	
    public:
        /// Constructor.
        AssetImporter();

        /// Destructor.
        virtual ~AssetImporter();

        /// Defines the base asset loading parameters.
        class Parameters
        {
        public:
            size_t priority = 0;
        };

        /// Get if given extension can be imported by this importer.
        virtual bool IsExtensionSupported(const std::string& extension) const = 0;

        virtual bool Import(const std::string& fileName, const std::string& destPath, std::shared_ptr<AssetImporter::Parameters> parameters) = 0;

    private:
        DISALLOW_COPY_MOVE_AND_ASSIGN(AssetImporter);
	};
}
