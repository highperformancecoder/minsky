import { existsSync, readFileSync, statSync } from 'fs';
import { dirname, join } from 'path';

function parseOsReleaseValue(osReleaseContent: string, key: string): string {
  const match=osReleaseContent.match(new RegExp(`^${key}=(.*)$`, 'm'));
  return match? match[1].trim().replace(/^['"]|['"]$/g, '').toLowerCase(): '';
}

export function requiresNoSandboxOnLinux(
  platform=process.platform,
  osReleaseContent?: string,
  hasUsableSandbox?: boolean
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
    if (hasUsableSandbox===undefined)
      hasUsableSandbox=hasUsableChromeSandbox();
    return distro==='ubuntu' && Number.isFinite(versionMajor) && versionMajor>=24 && !hasUsableSandbox;
  } catch {
    return false;
  }
}

function isSetuidRoot(path: string): boolean {
  try {
    const stat=statSync(path);
    return stat.uid===0 && (stat.mode & 0o4000)!==0;
  } catch {
    return false;
  }
}

export function hasUsableChromeSandbox(
  executablePath=process.execPath,
  resourcesPath=process.resourcesPath
): boolean {
  const candidates=[
    join(dirname(executablePath), 'chrome-sandbox'),
    join(resourcesPath, 'chrome-sandbox'),
  ];

  return candidates.some(path => existsSync(path) && isSetuidRoot(path));
}
