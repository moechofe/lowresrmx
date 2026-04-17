import SwiftUI

struct CommunityView: View {
    var body: some View {
        ScrollView {
					VStack(spacing: 24) {
                VStack(alignment: .leading, spacing: 8) {
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
                VStack(spacing: 16) {
                    Button("Community Website") {
                        UIApplication.shared.open(URL(string: "https://ret.ro.it")!)
                    }
                    .buttonStyle(.borderedProminent)
                    Button("Official Discord") {
                        UIApplication.shared.open(URL(string: "https://discord.gg/bNrNdRM8gT")!)
                    }
                    .buttonStyle(.bordered)
                    Button("Community Discord") {
                        UIApplication.shared.open(URL(string: "https://discord.gg/jcT9CXDgHB")!)
                    }
                    .buttonStyle(.bordered)
                }
                .padding(.bottom)
            }
        }
    }
}

struct CommunityView_Previews: PreviewProvider {
    static var previews: some View {
        CommunityView()
    }
}
