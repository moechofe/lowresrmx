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

import SwiftUI

struct CommunityView: View
{
	var body: some View
	{
		ScrollView
		{
			VStack(spacing: 24)
			{
				VStack(alignment: .leading, spacing: 8)
				{
					Text("“Fellow creators, the archives await. [Visit the community portal](https://ret.ro.it) to preserve your works and explore the legacy of programs recovered within.”")
						.font(.title2)
						.italic()
						.multilineTextAlignment(.leading)
						.padding(.horizontal)
					Text("— The_Digital_Archivist")
						.font(.subheadline)
						.foregroundColor(.secondary)
						.padding(.horizontal)
				}
				Spacer()
				VStack(spacing: 16)
				{
					Button("Community Website")
					{
						UIApplication.shared.open(URL(string: "https://ret.ro.it")!)
					}
					.buttonStyle(.borderedProminent)
					Button("Official Discord")
					{
						UIApplication.shared.open(URL(string: "https://discord.gg/bNrNdRM8gT")!)
					}
					.buttonStyle(.bordered)
					Button("Community Discord")
					{
						UIApplication.shared.open(URL(string: "https://discord.gg/jcT9CXDgHB")!)
					}
					.buttonStyle(.bordered)
				}
				.padding(.bottom)
			}
		}
	}
}

struct CommunityView_Previews: PreviewProvider
{
	static var previews: some View
	{
		CommunityView()
	}
}
