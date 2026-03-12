import type {ReactNode} from 'react';
import useDocusaurusContext from '@docusaurus/useDocusaurusContext';
import Layout from '@theme/Layout';

export default function Home(): ReactNode {
  const {siteConfig} = useDocusaurusContext();
  return (
    <Layout>
      <header className="text--center padding-vert--xl">
        <img className="only-on-light" style={{ margin: 'auto', padding: '1rem' }} src="/img/logo-light.png"/>
        <img className="only-on-dark" style={{ margin: 'auto', padding: '1rem' }} src="/img/logo-dark.png"/>
        <h1 className="hero__title">{siteConfig.title}</h1>
        <p className="hero__subtitle">{siteConfig.tagline}</p>
      </header>
      <main>
      </main>
    </Layout>
  );
}
