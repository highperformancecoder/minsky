import { existsSync, readFileSync } from 'fs';

function parseOsReleaseValue(osReleaseContent: string, key: string): string {
  const match=osReleaseContent.match(new RegExp(`^${key}=(.*)$`, 'm'));
  return match? match[1].trim().replace(/^['"]|['"]$/g, '').toLowerCase(): '';
}

export function requiresNoSandboxOnLinux(
  platform=process.platform,
  osReleaseContent?: string
): boolean {
  if (platform!=='linux') return false;
  try {
    if (!osReleaseContent) {
      let osRelease='/etc/os-release';
      if (existsSync(process.resourcesPath+'/os-release'))
        osRelease=process.resourcesPath+'/os-release';
      osReleaseContent=readFileSync(osRelease, 'utf8');
    }
    const distro=parseOsReleaseValue(osReleaseContent, 'ID');
    const versionId=parseOsReleaseValue(osReleaseContent, 'VERSION_ID');
    const versionMajor=Number(versionId.split('.')[0]);
    return distro==='ubuntu' && Number.isFinite(versionMajor) && versionMajor>=24;
  } catch {
    return false;
  }
}
