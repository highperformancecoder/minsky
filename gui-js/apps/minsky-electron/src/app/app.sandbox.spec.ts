import { requiresNoSandboxOnLinux } from './linux-sandbox';

describe('requiresNoSandboxOnLinux', () => {
  it('returns true for Ubuntu 24.04', () => {
    expect(requiresNoSandboxOnLinux('linux', 'ID=ubuntu\nVERSION_ID="24.04"\n')).toBe(true);
  });

  it('returns true for Ubuntu 26.04', () => {
    expect(requiresNoSandboxOnLinux('linux', 'ID=ubuntu\nVERSION_ID="26.04"\n')).toBe(true);
  });

  it('returns false for Ubuntu 22.04', () => {
    expect(requiresNoSandboxOnLinux('linux', 'ID=ubuntu\nVERSION_ID="22.04"\n')).toBe(false);
  });

  it('returns false for non-Ubuntu Linux', () => {
    expect(requiresNoSandboxOnLinux('linux', 'ID=debian\nVERSION_ID="12"\n')).toBe(false);
  });

  it('returns false for non-Linux platforms', () => {
    expect(requiresNoSandboxOnLinux('darwin', 'ID=ubuntu\nVERSION_ID="24.04"\n')).toBe(false);
  });
});
