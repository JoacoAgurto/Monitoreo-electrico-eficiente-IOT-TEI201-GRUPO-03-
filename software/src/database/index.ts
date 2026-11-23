
import { createClient } from "npm:@supabase/supabase-js@2";

const supabaseUrl = Deno.env.get("SUPABASE_URL")!;
const serviceKey = Deno.env.get("SUPABASE_SERVICE_ROLE_KEY")!;
const deviceToken = Deno.env.get("ESP32_DEVICE_TOKEN");

const supabase = createClient(supabaseUrl, serviceKey, {
  auth: {
    autoRefreshToken: false,
    persistSession: false,
  },
});

function responder(data: unknown, status = 200) {
  return new Response(JSON.stringify(data), {
    status,
    headers: { "Content-Type": "application/json" },
  });
}

function corrienteValida(valor: unknown): boolean {
  return valor === null ||
    (typeof valor === "number" &&
      Number.isFinite(valor) &&
      valor >= 0 &&
      valor <= 1000);
}

Deno.serve(async (req: Request) => {
  if (req.method !== "POST") {
    return responder({ error: "Metodo no permitido" }, 405);
  }

  if (!deviceToken) {
    return responder({ error: "Servidor sin configurar" }, 500);
  }

  const authorization = req.headers.get("Authorization") ?? "";
  const recibido = authorization.startsWith("Bearer ")
    ? authorization.slice(7)
    : "";

  // Comparacion en tiempo constante.
  const encoder = new TextEncoder();
  const esperadoBytes = encoder.encode(deviceToken);
  const recibidoBytes = encoder.encode(recibido);

  if (esperadoBytes.length !== recibidoBytes.length) {
    return responder({ error: "Token no autorizado" }, 401);
  }

  let diferencia = 0;
  for (let i = 0; i < esperadoBytes.length; i++) {
    diferencia |= esperadoBytes[i] ^ recibidoBytes[i];
  }

  if (diferencia !== 0) {
    return responder({ error: "Token no autorizado" }, 401);
  }

  const dispositivoHeader = req.headers.get("X-Device-ID");

  let body: Record<string, unknown>;

  try {
    body = await req.json();

    if (!body || Array.isArray(body) || typeof body !== "object") {
      return responder({ error: "JSON invalido" }, 400);
    }
  } catch {
    return responder({ error: "JSON invalido" }, 400);
  }

  const {
    dispositivo,
    fecha_hora,
    epoch_utc,
    conector_1,
    conector_2,
    conector_3,
    conector_4,
  } = body;

  if (
    typeof dispositivo !== "string" ||
    dispositivo.length < 1 ||
    dispositivo.length > 64 ||
    dispositivo !== dispositivoHeader ||
    typeof fecha_hora !== "string" ||
    !Number.isInteger(epoch_utc) ||
    typeof epoch_utc !== "number" ||
    epoch_utc < 1700000000 ||
    epoch_utc > 4102444800 ||
    !Number.isFinite(Date.parse(fecha_hora)) ||
    Math.abs(Date.parse(fecha_hora) / 1000 - epoch_utc) > 1 ||
    ![conector_1, conector_2, conector_3, conector_4]
      .every(corrienteValida)
  ) {
    return responder({ error: "Datos invalidos" }, 400);
  }

  const { error } = await supabase
    .from("mediciones")
    .upsert(
      {
        dispositivo,
        fecha_hora,
        epoch_utc,
        conector_1,
        conector_2,
        conector_3,
        conector_4,
      },
      { onConflict: "dispositivo,epoch_utc" },
    );

  if (error) {
    console.error("Error de base de datos:", error.message);
    return responder({ error: "Error guardando medicion" }, 500);
  }

  return responder({
    ok: true,
    mensaje: "Medicion recibida",
    dispositivo,
    epoch_utc,
  });
});
