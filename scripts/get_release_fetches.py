import pandas as pd
import requests

resp = requests.get('https://api.github.com/repos/arkottke/strata/releases')
data = resp.json()

records = []
for d in data:
    for a in d['assets']:
        records.append(
            (d['tag_name'], d['published_at'], a['name'], a['download_count'])
        )

df = pd.DataFrame(
    records,
    columns=['tag', 'published_at', 'name', 'download_count']
)

df.to_csv('release_fetches.csv', index=False)
