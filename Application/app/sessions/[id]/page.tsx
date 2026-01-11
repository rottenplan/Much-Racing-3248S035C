import fs from 'fs';
import path from 'path';
import SessionView from './SessionView';

async function getSessionData(id: string) {
  const sessionsDir = path.join(process.cwd(), 'data', 'sessions');
  const files = fs.readdirSync(sessionsDir);

  // Find file containing ID or matching timestamp (ID is timestamp)
  // Simplified matching: look for file ending with `_${id}.json` or just containing ID
  const file = files.find(f => f.includes(`_${id}.json`) || f.includes(id));

  if (!file) return null;

  try {
    const content = fs.readFileSync(path.join(sessionsDir, file), 'utf-8');
    return JSON.parse(content);
  } catch (e) {
    console.error(e);
    return null;
  }
}

export default async function SessionDetailPage({ params }: { params: { id: string } }) {
  const session = await getSessionData(params.id);

  if (!session) {
    return (
      <div className="min-h-screen bg-slate-900 flex items-center justify-center text-white">
        Session not found (ID: {params.id})
      </div>
    );
  }

  return <SessionView session={session} />;
}
