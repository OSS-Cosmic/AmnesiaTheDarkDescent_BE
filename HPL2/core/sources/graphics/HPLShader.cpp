#include "graphics/HPLShader.h"
#include <Common_3/Renderer/IResourceLoader.h>

hpl::HPLShader::HPLShader(Renderer* renderer, const ShaderMember stages[], size_t numStages, uint32_t features) {
  ShaderLoadDesc desc = {};
  std::vector<ShaderMacro> targetMacros[SHADER_STAGE_COUNT];
  for (int i = 0; i < numStages; ++i) {
    const ShaderMember &member = stages[i];
    desc.mStages[i].pFileName = member.name;
    for (int st = 0; st < stages[i].permutationCount; st++) {
      if ((member.permutations->bits & features) > 0 ||
          member.permutations->bits == 0) {
        targetMacros[i].push_back({.definition = member.permutations->key,
                                   .value = member.permutations->value});
      }
    }
    desc.mStages[i].pMacros = targetMacros[i].data();
    desc.mStages[i].mMacroCount = targetMacros[i].size();
  }
  addShader(renderer, &desc, &_shader);
}

hpl::HPLShader::~HPLShader() {

}