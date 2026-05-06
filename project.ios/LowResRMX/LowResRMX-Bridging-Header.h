// Copyright 2016-2024 Timo Kloss
// Copyright 2021-2026 Martin Mauchauffée

// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.

// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:

// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "core.h"
#include "core_stats.h"

#import "HelpSplitViewController.h"
#import "HelpContent.h"
#import "AppStyle.h"
#import "NSString+Utils.h"
#import "UITextView+Utils.h"
#import "UIViewController+LowResCoder.h"

#import "EditorTextView.h"
#import "SearchToolbar.h"
#import "BlockerView.h"
#import "GORLabel.h"
