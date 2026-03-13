import type {ReactNode} from 'react';
import useDocusaurusContext from '@docusaurus/useDocusaurusContext';
import useBaseUrl from '@docusaurus/useBaseUrl';
import Layout from '@theme/Layout';

export default function Home(): ReactNode {
  const {siteConfig} = useDocusaurusContext();
  const logoLight = useBaseUrl('/img/logo-light.png');
  const logoDark = useBaseUrl('/img/logo-dark.png');

  return (
    <Layout>
      <header className="text--center padding-vert--xl">
        <img className="only-on-light" style={{ margin: 'auto', padding: '1rem' }} src={logoLight} alt="Nebula logo (light)" />
        <img className="only-on-dark" style={{ margin: 'auto', padding: '1rem' }} src={logoDark} alt="Nebula logo (dark)" />
        <h1 className="hero__title">{siteConfig.title}</h1>
        <p className="hero__subtitle">{siteConfig.tagline}</p>
      </header>
      <main>
      </main>
    </Layout>
  );
}
