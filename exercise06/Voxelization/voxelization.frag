// Extension aktivieren, damit << im Befehlssatz vorliegt.
#extension GL_EXT_gpu_shader4 : enable

// Ausgabevariable
varying out uvec4 result;

void main()
{	
	// TODO: Tiefenwert von [0..1] auf {0..127} abbilden.
  unsigned int depth = unsigned int(gl_FragCoord.z * 127.);
	
	// Dies ergibt beispielsweise den Wert 42.
	// Erzeugen Sie nun eine bit-Maske, in der das (im Beispiel) 42te Bit (von rechts gezählt) eine 1 ist und alle anderen eine 0.
	// 00000000..000000010000000..00000000
	// |<- 86 Nullen ->| |<- 41 Nullen ->|
	//                  ^
	//                Bit 42
	// Weisen Sie diese bit-Maske der Variable 'result' zu.
  
  if (depth < 32u)
  {
    result.w = ~0u << depth;
    result.z = ~0u;
    result.y = ~0u;
    result.x = ~0u;
  }
  else if (depth < 64u)
  {
    result.z = ~0u << (depth % 32u);
    result.y = ~0u;
    result.x = ~0u;
  }
  else if (depth < 96u)
  {
    result.y = ~0u << (depth % 32u);
    result.x = ~0u;
  }
  else if (depth < 128u)
  {
    result.x = ~0u << (depth % 32u);
  }
  
}
