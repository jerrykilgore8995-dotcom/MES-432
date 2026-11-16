import pathlib, urllib.request, re, html, urllib.parse, hashlib, json
out=pathlib.Path("reference-downloads");out.mkdir(exist_ok=True)
sources={"audio-board.gif":"https://gyraf.dk/schematics/audioboard432.gif","hardware-1k-Q5-boosts.png":"https://phillipdempster.com/post/the-sound-of-sontec/1k%20Q5%20boosts_432.png","hardware-panel.jpg":"https://phillipdempster.com/post/the-sound-of-sontec/featured.jpg","measurement-article.html":"https://phillipdempster.com/post/the-sound-of-sontec/","250-reference.pdf":"https://thedonclassics.com/250eq/schematic/250eq-diy-schematic.pdf"}
manifest=[]
def fetch(name,url):
 try:
  req=urllib.request.Request(url,headers={"User-Agent":"Mozilla/5.0"})
  with urllib.request.urlopen(req,timeout=30) as r:data=r.read();typ=r.headers.get("Content-Type","")
  if name.endswith((".png",".gif",".jpg")) and not typ.startswith("image/"):raise ValueError(typ)
  (out/name).write_bytes(data)
  manifest.append(dict(file=name,url=url,sha256=hashlib.sha256(data).hexdigest(),bytes=len(data)))
  return data
 except Exception as e:manifest.append(dict(file=name,url=url,error=str(e)));return b""
for name,url in sources.items():
 data=fetch(name,url)
 if name=="measurement-article.html":
  links=set(re.findall(r'(?:src|href)=["\']([^"\']+\.(?:png|jpg|jpeg|webp)(?:\?[^"\']*)?)["\']',data.decode(errors="replace"),re.I))
  for i,link in enumerate(sorted(links)):
   u=urllib.parse.urljoin(url,html.unescape(link));ext=pathlib.Path(urllib.parse.urlsplit(u).path).suffix
   fetch(f"article-image-{i:03d}{ext}",u)
(out/"manifest.json").write_text(json.dumps(manifest,indent=2))
print(json.dumps(manifest,indent=2))
