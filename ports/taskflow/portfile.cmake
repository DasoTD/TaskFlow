vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO DasoTD/TaskFlow
    REF "v${VERSION}"
    SHA512 0  # This will be filled in by vcpkg when the port is created
    HEAD_REF main
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DTASKFLOW_BUILD_TESTS=OFF
        -DTASKFLOW_BUILD_EXAMPLES=OFF
        -DTASKFLOW_BUILD_BENCHMARKS=OFF
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/taskflow)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)