def can_build(env, platform):
    if env["vulkan"]:
        return False
    return True

def configure(env):
    pass

def get_doc_classes():
    return [
        "MobileVRInterface",
    ]

def get_doc_path():
    return "doc_classes"
