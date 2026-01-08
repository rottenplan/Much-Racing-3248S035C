import { NextResponse } from 'next/server';

// Simple in-memory store for demo purposes
// In production, this would query a database
const mockUserSettings = {
    settings: {
        units: 'kmh',
        temperature: 'celsius',
        gnss: 'gps',
        brightness: 50,
        powerSave: 5,
        contrast: 50
    },
    tracks: {
        countries: ['Indonesia', 'Malaysia', 'Singapore'],
        trackCount: 25
    },
    engines: [
        { id: 1, name: 'Much Racing', hours: 24.5 },
        { id: 2, name: 'Engine 2', hours: 0.0 },
        { id: 3, name: 'Engine 3', hours: 0.0 }
    ],
    activeEngine: 1
};

export async function GET(request: Request) {
    try {
        // Basic authentication check
        const authHeader = request.headers.get('authorization');

        if (!authHeader || !authHeader.startsWith('Basic ')) {
            return NextResponse.json(
                { error: 'Unauthorized - Missing credentials' },
                { status: 401 }
            );
        }

        // Decode Basic Auth (format: "Basic base64(username:password)")
        const base64Credentials = authHeader.split(' ')[1];
        const credentials = Buffer.from(base64Credentials, 'base64').toString('ascii');
        const [username, password] = credentials.split(':');

        // Simple validation (in production, check against database)
        if (!username || !password) {
            return NextResponse.json(
                { error: 'Unauthorized - Invalid credentials' },
                { status: 401 }
            );
        }

        // TODO: Validate credentials against database
        // For now, accept any non-empty credentials

        // Return user's device settings and track selection
        return NextResponse.json({
            success: true,
            data: mockUserSettings,
            syncTime: new Date().toISOString()
        });

    } catch (error) {
        console.error('Sync API Error:', error);
        return NextResponse.json(
            { error: 'Internal server error' },
            { status: 500 }
        );
    }
}

// POST endpoint to update settings from device
export async function POST(request: Request) {
    try {
        // Basic authentication check
        const authHeader = request.headers.get('authorization');

        if (!authHeader || !authHeader.startsWith('Basic ')) {
            return NextResponse.json(
                { error: 'Unauthorized - Missing credentials' },
                { status: 401 }
            );
        }

        const body = await request.json();

        // In production, save to database
        // For now, just acknowledge receipt
        console.log('Settings update from device:', body);

        return NextResponse.json({
            success: true,
            message: 'Settings updated successfully',
            syncTime: new Date().toISOString()
        });

    } catch (error) {
        console.error('Sync API Error:', error);
        return NextResponse.json(
            { error: 'Internal server error' },
            { status: 500 }
        );
    }
}
