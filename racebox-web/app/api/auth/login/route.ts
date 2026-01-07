import { NextResponse } from 'next/server';

export async function POST(request: Request) {
  try {
    const body = await request.json();
    const { email, password } = body;

    // TODO: Connect to real database
    // For now, simple mock authentication
    if (email === 'user@example.com' && password === 'password123') {
      return NextResponse.json({
        success: true,
        token: 'mock_jwt_token_1234567890',
        user: {
          id: 1,
          name: 'Demo User',
          email: 'user@example.com'
        }
      });
    }

    // Allow any "valid-looking" login for testing purposes if not specific mock
    if (password && password.length >= 6) {
         return NextResponse.json({
            success: true,
            token: 'mock_jwt_token_' + Date.now(),
            user: {
              id: Math.floor(Math.random() * 1000),
              name: 'Test User',
              email: email
            }
          });
    }

    return NextResponse.json(
      { success: false, message: 'Invalid credentials' },
      { status: 401 }
    );
  } catch (error) {
    return NextResponse.json(
      { success: false, message: 'Internal server error' },
      { status: 500 }
    );
  }
}
