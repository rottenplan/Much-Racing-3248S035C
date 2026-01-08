import { NextResponse } from 'next/server';
import fs from 'fs';
import path from 'path';

export async function POST(request: Request) {
    try {
        const body = await request.json();
        const { trackName, startLine, points, country, trackType } = body;

        if (!trackName || !startLine) {
            return NextResponse.json(
                { error: 'Missing required fields' },
                { status: 400 }
            );
        }

        const dataDir = path.join(process.cwd(), 'data', 'tracks');
        if (!fs.existsSync(dataDir)) {
            fs.mkdirSync(dataDir, { recursive: true });
        }

        // Create sanitized filename
        const safeName = trackName.replace(/[^a-zA-Z0-9]/g, '_').toLowerCase();
        const timestamp = Date.now();
        const fileName = `${safeName}_${timestamp}.json`;
        const filePath = path.join(dataDir, fileName);

        const trackData = {
            id: timestamp.toString(),
            name: trackName,
            country: country || 'Unknown',
            type: trackType || 'Circuit',
            startLine: startLine, // { lat, lng, bearing, width }
            points: points || [],
            createdAt: new Date().toISOString()
        };

        fs.writeFileSync(filePath, JSON.stringify(trackData, null, 2));

        return NextResponse.json({
            success: true,
            message: 'Track saved successfully',
            file: fileName
        });

    } catch (error) {
        console.error('Error saving track:', error);
        return NextResponse.json(
            { error: 'Internal Server Error' },
            { status: 500 }
        );
    }
}
