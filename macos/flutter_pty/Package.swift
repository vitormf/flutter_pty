// swift-tools-version: 5.9
import PackageDescription

let package = Package(
    name: "flutter_pty",
    platforms: [
        .macOS("10.14")
    ],
    products: [
        .library(name: "flutter-pty", targets: ["flutter_pty"])
    ],
    targets: [
        .target(
            name: "flutter_pty",
            path: "Classes",
            publicHeadersPath: "."
        )
    ]
)
