
import { NextRequest, NextResponse } from 'next/server';
import { parseGpx } from '@/lib/gpxParser';
import fs from 'fs';
import path from 'path';

export async function POST(req: NextRequest) {
    try {
        const formData = await req.formData();
        const file = formData.get('file') as File;

        if (!file) {
            return NextResponse.json({ error: 'No file uploaded' }, { status: 400 });
        }

        const buffer = Buffer.from(await file.arrayBuffer());
        const gpxContent = buffer.toString('utf-8');

        // Parse the GPX content
        const parsedData = parseGpx(gpxContent);

        // Create 'data/sessions' directory if it doesn't exist
        const sessionsDir = path.join(process.cwd(), 'data', 'sessions');
        if (!fs.existsSync(sessionsDir)) {
            fs.mkdirSync(sessionsDir, { recursive: true });
        }

        // Create a new session object
        const sessionId = Date.now().toString();
        const newSession = {
            id: sessionId,
            originalFilename: file.name,
            uploadDate: new Date().toISOString(),
            trackName: parsedData.name,
            stats: {
                totalPoints: parsedData.points.length,
                maxSpeed: parsedData.points.reduce((max, p) => Math.max(max, p.speed || 0), 0) * 3.6, // m/s to km/h usually
                startTime: parsedData.points[0]?.time,
                endTime: parsedData.points[parsedData.points.length - 1]?.time
            },
            points: parsedData.points
        };

        // Save to JSON file (simple mock DB)
        const filePath = path.join(sessionsDir, `${sessionId}.json`);
        fs.writeFileSync(filePath, JSON.stringify(newSession, null, 2));

        return NextResponse.json({ success: true, sessionId, message: 'GPX parsed and saved' });

    } catch (error) {
        console.error('GPX Upload Error:', error);
        return NextResponse.json({ error: 'Failed to process GPX file' }, { status: 500 });
    }
}
