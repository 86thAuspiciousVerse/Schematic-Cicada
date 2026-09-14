import { parseEngineLine, resolveEdge } from '../../cicada-harness/packages/cicada/cicada-launcher/src/launcher.ts'
console.log('parse:', JSON.stringify(parseEngineLine('cicada-engine: 127.0.0.1:59974 e77358de36a8a96d667fa11da03a13b2')))
console.log('edge:', resolveEdge())
