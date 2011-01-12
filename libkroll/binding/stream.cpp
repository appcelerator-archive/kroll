/*
 * Copyright (c) 2011 Appcelerator, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "../kroll.h"

namespace kroll {

Stream::Stream(const char* type) : StaticBoundObject(type)
{
}

Stream::~Stream()
{
}

void Stream::Write(const char* buffer, size_t size)
{
    throw ValueException::FromString("Stream is not writable.");
}

bool Stream::IsWritable() const
{
    return false;
}

size_t Stream::Read(const char* buffer, size_t size)
{
    throw ValueException::FromString("Stream is not readable.");
}

bool Stream::IsReadable() const
{
    return false;
}

} // namespace kroll
