import {themes as prismThemes} from 'prism-react-renderer';
import type {Config} from '@docusaurus/types';
import type * as Preset from '@docusaurus/preset-classic';

const config: Config = {
  title: 'Nebula Docs',
  tagline: 'The visual, high-performance, community-driven C++ editor.',
  favicon: 'img/favicon.ico',

  future: {
    v4: true,
  },

  url: 'https://nebula-eip.github.io',
  baseUrl: '/POC-App/',

  organizationName: 'Nebula-EIP',
  projectName: 'POC-App',

  onBrokenLinks: 'throw',

  i18n: {
    defaultLocale: 'en',
    locales: ['en'],
  },

  presets: [
    [
      'classic',
      {
        docs: {
          sidebarPath: './sidebars.ts',
        },
        blog: false,
        theme: {
          customCss: './src/css/custom.css',
        },
      } satisfies Preset.Options,
    ],
  ],

  themeConfig: {
    image: 'img/nebula-social-card.jpg',
    colorMode: {
      respectPrefersColorScheme: true,
    },
    navbar: {
      logo: {
        alt: 'Nebula Logo',
        src: 'img/nebula-light.png',
        srcDark: 'img/nebula-dark.png',
      },
      items: [
        {
          type: 'docSidebar',
          sidebarId: 'aboutSidebar',
          position: 'left',
          label: 'About Nebula',
        },
        {
          type: 'docSidebar',
          sidebarId: 'userSidebar',
          position: 'left',
          label: 'Users',
        },
        {
          type: 'docSidebar',
          sidebarId: 'devSidebar',
          position: 'left',
          label: 'Developers',
        },
        {
          href: 'pathname:///api/index.html',
          label: 'API Reference',
          position: 'left',
        },
      ],
    },
    footer: {
      style: 'dark',
      links: [
        {
          title: 'About Nebula',
          items: [
            {
              label: 'Introduction',
              to: '/docs/about/intro',
            },
          ],
        },
        {
          title: 'Users',
          items: [
            {
              label: 'User Guide',
              to: '/docs/user/intro',
            },
          ],
        },
        {
          title: 'Developers',
          items: [
            {
              label: 'Developer Guide',
              to: '/docs/dev/intro',
            },
          ],
        },
        {
          title: 'API Reference',
          items: [
            {
              label: 'Doxygen API',
              href: 'pathname:///api/index.html',
            },
          ],
        },
      ],
      copyright: `Copyright © ${new Date().getFullYear()} Nebula.`,
    },
    prism: {
      theme: prismThemes.github,
      darkTheme: prismThemes.dracula,
    },
  } satisfies Preset.ThemeConfig,
};

export default config;
